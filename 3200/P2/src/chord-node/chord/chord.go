package chord

import (
	"crypto/sha1"
	"encoding/binary"
	"log"
	"math"
	"net"
	"net/http"
	"net/rpc"
	"strconv"
	"strings"
	"sync"
	"time"
)

// NodeInfo
type NodeInfo struct {
	Name string
	ID   uint64
}

// M is a constant
const M = 16

// MAXNODES is a comment
var MAXNODES = uint64(math.Pow(2, M))

// Node
type Node struct {
	Successor   NodeInfo
	Predecessor NodeInfo
	NodeInfo

	FingerTable []NodeInfo
	Intervall   time.Duration
	mux         sync.Mutex
	data        map[uint64][]byte
	update      bool
}

// New is a function
func New(name string) (*Node, error) {
	// hash name
	h := sha1.New()
	h.Write([]byte(name))
	cs := h.Sum(nil)

	node := new(Node)
	node.Name = name
	node.ID = binary.BigEndian.Uint64(cs) % MAXNODES
	node.Intervall = 1000
	node.FingerTable = make([]NodeInfo, M)
	node.update = true
	node.data = make(map[uint64][]byte)

	//To able to perform RPC a node must listen to a port
	rpc.Register(node)
	rpc.HandleHTTP()
	l, err := net.Listen("tcp", name[strings.Index(name, ":"):])

	if err != nil {
		return nil, err
	}

	go node.Stabilize()
	go node.CheckPredecessor()
	go http.Serve(l, nil)

	return node, nil
}

/*
 * CHORD INTERFACE
 */

// Create a new network
func (node *Node) Create() {
	node.mux.Lock()
	node.Successor.ID = node.ID
	node.Successor.Name = node.Name
	node.mux.Unlock()
}

// Join connects to an exsisting network
func (node *Node) Join(name string) error {
	// Fetchning Successor
	s, err := FindSuccessor(node.ID, name)

	if err != nil {
		return err
	}

	// Linking Successor
	node.mux.Lock()
	node.Successor.ID = s.ID
	node.Successor.Name = s.Name
	node.mux.Unlock()

	return nil
}

//ClosestPrecedingNode search the finger table for closest node
func (node *Node) ClosestPrecedingNode(id uint64) string {
	//Finger table may be uninitialized
	if node.FingerTable[0].Name == "" {
		return node.Successor.Name
	}
	// Iterating through the finger table to find the closest node.
	for i := M - 1; i >= 0; i-- {
		if ElementIn(node.FingerTable[i].ID, node.ID, id) {
			return node.FingerTable[i].Name
		}
	}

	return node.Successor.Name
}

//Close is a function
func (node *Node) Close() error {
	node.update = false
	p, err := NodeGet(node.Predecessor.Name)
	if err != nil {
		return err
	}

	s, err := NodeGet(node.Successor.Name)
	if err != nil {
		return err
	}

	s.Predecessor.Name = p.Name
	s.Predecessor.ID = p.ID
	err = NodeSet(s.Name, s)
	if err != nil {
		return err
	}

	p.Successor.Name = s.Name
	p.Successor.ID = s.ID
	err = NodeSet(p.Name, p)
	if err != nil {
		return err
	}
	return nil
}

// Stabilize ping its successor to check the successor.Predecessor is itself.
func (node *Node) Stabilize() error {
	for node.update {
		// Sleep for a given intervall
		time.Sleep(node.Intervall * time.Millisecond)
		var s *Node

		// Fetch successor
		s, err := NodeGet(node.Successor.Name)
		if err != nil {
			log.Fatal("Couldn't fetch successor")
		}

		// Check if successor -> Predecessor is me
		node.mux.Lock()
		if ElementIn(s.Predecessor.ID, node.ID, s.ID) && s.Predecessor.Name != "" {
			node.Successor.Name = s.Predecessor.Name
			node.Successor.ID = s.Predecessor.ID
		}
		node.mux.Unlock()

		// Always notify successor
		Notify(node.Successor.Name, node)
	}
	return nil
}

//FixFingers updates the finger table as node joisn and leaves
func (node *Node) FixFingers() error {
	for node.update {
		time.Sleep(node.Intervall * 5 * time.Millisecond)
		for next := 0; next < M; next++ {
			// Calculate requesting id
			id := (node.ID + uint64(math.Pow(2, float64(next)))) % MAXNODES

			// Find the successor to the Calculated id
			n, err := FindSuccessor(id, node.Successor.Name)

			if err != nil {
				log.Fatal("Something went wrong while fixing fingers")
			}
			// Update entries
			node.FingerTable[next].ID = n.ID
			node.FingerTable[next].Name = n.Name
		}
	}
	return nil
}

//CheckPredecessor ping its Predecessor to check if it still alive.
func (node *Node) CheckPredecessor() error {
	for node.update {
		time.Sleep(node.Intervall * time.Millisecond)
		// try to fetch the Predecessor
		p, err := NodeGet(node.Predecessor.Name)
		if err != nil && p == nil {
			node.Predecessor.Name = ""
			node.Predecessor.ID = 0
		}
	}
	return nil
}

//SetIntervall is a function
func (node *Node) SetIntervall(intervall time.Duration) {
	node.Intervall = intervall
}

// FindSuccessor find the successor to the given ID
func FindSuccessor(id uint64, name string) (*Node, error) {

	n, err := rpc.DialHTTP("tcp", name)
	if err != nil {
		return nil, err
	}

	// Perform RPC call on given node.
	reply := new(Node)
	err = n.Call("Node.RPCFindSuccessor", id, reply)
	if err != nil {
		n.Close()
		return nil, err
	}

	err = n.Close()
	if err != nil {
		return nil, err
	}

	return reply, nil
}

//Notify will try to notify a nodes successor to update it's Predecessor
func Notify(name string, node *Node) error {
	n, err := rpc.DialHTTP("tcp", name)
	if err != nil {
		return err
	}

	err = n.Call("Node.RPCNotify", node, node)
	if err != nil {
		n.Close()
		return err
	}
	err = n.Close()
	if err != nil {
		return err
	}
	return nil
}

func (node *Node) Get(k string) (*[]byte, error) {
	h := sha1.New()
	h.Write([]byte(k))
	cs := h.Sum(nil)
	key := binary.BigEndian.Uint64(cs) % MAXNODES

	n, err := FindSuccessor(key, node.Name)
	if err != nil {
		return nil, err
	}

	value, err := getData(n.Name, key)
	if err != nil {
		return nil, err
	}

	return value, nil
}

func (node *Node) Put(k string, value []byte) error {
	h := sha1.New()
	h.Write([]byte(k))
	cs := h.Sum(nil)
	key := binary.BigEndian.Uint64(cs) % MAXNODES

	n, err := FindSuccessor(key, node.Name)
	if err != nil {
		return err
	}

	err = putData(n.Name, key, value)
	if err != nil {
		return err
	}

	return nil
}

/*
 * HELPER FUNCTIONS
 */

type MapPkg struct {
	Key   uint64
	Value []byte
}

func putData(name string, key uint64, value []byte) error {
	node, err := rpc.DialHTTP("tcp", name)
	if err != nil {
		return err
	}
	pkg := new(MapPkg)
	pkg.Key = key
	pkg.Value = value

	err = node.Call("Node.RPCDataPut", pkg, pkg)
	if err != nil {
		return err
	}
	err = node.Close()
	if err != nil {
		return err
	}

	return nil
}

func getData(name string, key uint64) (*[]byte, error) {
	node, err := rpc.DialHTTP("tcp", name)
	if err != nil {
		return nil, err
	}

	value := new([]byte)
	err = node.Call("Node.RPCDataGet", key, value)
	if err != nil {
		return nil, err
	}
	err = node.Close()
	if err != nil {
		return nil, err
	}

	return value, nil
}

func (node *Node) RPCDataPut(pkg *MapPkg, ignore *MapPkg) error {
	node.data[pkg.Key] = pkg.Value
	return nil
}

func (node *Node) RPCDataGet(key uint64, value *[]byte) error {
	*value = node.data[key]
	return nil
}

//NodeGet Retrieves a node
func NodeGet(name string) (*Node, error) {
	node, err := rpc.DialHTTP("tcp", name)
	if err != nil {
		return nil, err
	}

	n := new(Node)
	err = node.Call("Node.RPCNodeGet", n, n)
	if err != nil {
		node.Close()
		return nil, err
	}

	err = node.Close()
	if err != nil {
		return nil, err
	}

	return n, nil
}

//SetNode is a function
func NodeSet(name string, n *Node) error {
	node, err := rpc.DialHTTP("tcp", name)
	if err != nil {
		return err
	}

	err = node.Call("Node.RPCNodeSet", n, n)
	if err != nil {
		node.Close()
		return err
	}

	err = node.Close()
	if err != nil {
		return err
	}
	return nil
}

//NodeCopy copies a = b
func NodeCopy(a *Node, b *Node) {
	a.Name = b.Name
	a.ID = b.ID

	a.Successor.Name = b.Successor.Name
	a.Successor.ID = b.Successor.ID

	a.Predecessor.Name = b.Predecessor.Name
	a.Predecessor.ID = b.Predecessor.ID
}

//ElementIn return true if a is between pre and suc. else return false
func ElementIn(a uint64, pre uint64, suc uint64) bool {
	if pre < a && a < suc {
		return true
	}
	if pre == suc {
		return true
	}
	if (pre > suc) && (a > pre || a < suc) {
		return true
	}
	return false
}

/*
 * RPC CALLS
 */

//RPCNodeGet retrieves node
func (node *Node) RPCNodeGet(n *Node, reply *Node) error {
	NodeCopy(reply, node)
	return nil
}

//RPCNodeSet retrieves node
func (node *Node) RPCNodeSet(n *Node, reply *Node) error {
	node.mux.Lock()
	node.Predecessor.Name = n.Predecessor.Name
	node.Predecessor.ID = n.Predecessor.ID
	node.Successor.Name = n.Successor.Name
	node.Successor.ID = n.Successor.ID
	node.mux.Unlock()
	return nil
}

//RPCFindSuccessor finds the Successor of the given id.
func (node *Node) RPCFindSuccessor(id uint64, reply *Node) error {
	Successor, err := NodeGet(node.Successor.Name)
	if err != nil {
		return err
	}
	// check if id exist in current node and it's Successor
	if ElementIn(id, node.ID, Successor.ID) {
	} else {
		n, err := rpc.DialHTTP("tcp", node.Successor.Name)
		if err != nil {
			return err
		}
		err = n.Call("Node.RPCFindSuccessor", id, reply)
		if err != nil {
			n.Close()
			return err
		}

		err = n.Close()
		if err != nil {
			return err
		}
		return nil
	}

	// copies Successor to reply
	NodeCopy(reply, Successor)

	return nil

}

//RPCNotify checks Successor need to update its Predecessor
func (node *Node) RPCNotify(n *Node, reply *Node) error {
	node.mux.Lock()
	if node.Predecessor.Name == "" || ElementIn(n.ID, node.Predecessor.ID, node.ID) {
		node.Predecessor.Name = n.Name
		node.Predecessor.ID = n.ID
	}
	node.mux.Unlock()
	return nil
}

/*
 * Helper functions for the chord-node
 */

//GetN is a function
func (node *Node) GetN() []string {
	var t []string
	t = append(t, PortDecrement(node.Successor.Name))
	if node.Predecessor.Name != "" {
		t = append(t, PortDecrement(node.Predecessor.Name))
	}
	return t
}

func portHandler(hostname string, t int) string {
	host := hostname[:strings.Index(hostname, ":")]
	port := hostname[strings.Index(hostname, ":")+1:]
	newPort, err := strconv.Atoi(port)
	if err != nil {
		return ""
	}
	newPort = newPort + t
	return host + ":" + strconv.Itoa(newPort)
}

func PortIncrement(hostname string) string {
	return portHandler(hostname, 1)
}

func PortDecrement(hostname string) string {
	return portHandler(hostname, -1)
}
