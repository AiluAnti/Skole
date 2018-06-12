package main

import (
	"bytes"
	"crypto/sha1"
	"encoding/binary"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"log"
	"math"
	"math/rand"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"sync"
	"time"

	"../config"
)

var hostname string
var port string
var maxRunTime time.Duration
var exitReason chan string

var conf config.Config

var neighbours []string
var neighboursLock sync.RWMutex

var MAXNUM = uint64(math.Pow(2, 32))

//node structure
type Node struct {
	predecessor NodeName
	successor   NodeName
	NodeName
}

// Struct describing nodes name
type NodeName struct {
	ID   uint64
	Name string
}

var gNode Node

// Create new ring
/* At this point it only needs to create a new node.
// There is no difference between a new node and a new ring
// Before joining a network
*/
func createRing() {
	log.Printf("Setting up new Chord Ring......")
	// Creating the new node by using the set structs
	gNode.successor.Name = gNode.Name
	gNode.predecessor.Name = gNode.Name
}

func main() {
	// Get variables from environment
	hostname = config.Hostname()

	// Set random seed
	rand.Seed(time.Now().UTC().UnixNano())

	// Parse command line params
	flag.StringVar(&port, "p", ":8181", "node port (prefix with colon)")

	var joinPeer string
	flag.StringVar(&joinPeer, "join", "", "Existing peer in network to contact for join (host:port)")

	flag.Parse()

	// Set log prefix
	log.SetPrefix(hostname + port + ": ")
	gNode.Name = hostname + port

	//hash new node
	h := sha1.New()
	h.Write([]byte(gNode.Name))
	checksum := h.Sum(nil)
	gNode.ID = binary.BigEndian.Uint64(checksum) % MAXNUM

	// Get configuration
	var err error
	conf, err = config.ReadConfig()
	if err != nil {
		log.Fatalf("Error getting config: %s", err)
	}
	maxRunTime, err = time.ParseDuration(conf.MaxRun)
	if err != nil {
		log.Fatalf("Invalid MaxRun duration in config: %s: %s", conf.MaxRun, err)
	}

	// Set up a channel to trigger shut down (send a string saying why)
	exitReason = make(chan string, 1)
	go func() {
		reason := <-exitReason
		log.Print("Shutting down: " + reason)
		os.Exit(0)
	}()

	// Shut down after maxRunTime timeout
	go func() {
		time.Sleep(maxRunTime)
		exitReason <- fmt.Sprintf("maxrun timeout: %s", maxRunTime)
	}()

	// Attempt to join network
	neighbours = []string{}
	if joinPeer != "" {
		err = joinNetwork(joinPeer)
		if err != nil {
			log.Fatal("Fatal: Could not join network: ", err)
		}
	} else {
		log.Print("No peers to join. New network")
		createRing()
	}

	// Set up http handler paths
	http.HandleFunc("/", indexHandler)
	http.HandleFunc("/addNode", addNodeHandler)
	http.HandleFunc("/shutdown", shutdownHandler)
	http.HandleFunc("/neighbours", neighboursHandler)
	http.HandleFunc("/joinnetwork", joinNetworkHandler)
	http.HandleFunc("/notify", notifyHandler)
	http.HandleFunc("/findPredecessor", findPredecessorHandler)
	// Start server
	log.Printf("Starting node on %s%s\n", hostname, port)

	// stabilize routinely
	go func() {
		for {
			time.Sleep(1000 * time.Millisecond)
			stabilize()
		}
	}()
	err = http.ListenAndServe(port, nil)

	if err != nil {
		log.Fatal(err)
	}
}

func indexHandler(w http.ResponseWriter, r *http.Request) {

	// Send response
	body := "Node running on " + hostname + port + "\n"
	fmt.Fprintf(w, body)
}

func addNodeHandler(w http.ResponseWriter, r *http.Request) {

	// Only accept POST
	if r.Method != "POST" {
		http.Error(w, "Method not allowed", 405)
		return
	}

	newNode, err := launchRandomNode()
	if err != nil {
		log.Println(err)
		http.Error(w, err.Error(), 500)
		return
	}
	fmt.Fprintln(w, newNode)
}

// Launch a new node at a random host and port taken from the config
//
// Note: This function will start the process and return immediately. If the
// process fails, you will not get feedback for that here. Unfortunately there
// is not an easy way to get that feedback.
//
// Perhaps you could start a go routine that waits for a ping from the server
// so that you know it is up?
func launchRandomNode() (string, error) {
	// Find current executable path
	execpath, err := filepath.Abs(os.Args[0])
	if err != nil {
		return "", err
	}

	// Pick a random host from config
	randHost := conf.Hosts[rand.Intn(len(conf.Hosts))]

	// Pick a random port in configured range
	randPort := conf.NodeLowPort + rand.Intn(conf.NodeHighPort-conf.NodeLowPort+1)
	portStr := fmt.Sprintf(":%d", randPort)

	joinStr := fmt.Sprintf(hostname + port)

	// Build a command line
	commandLine := []string{execpath, "-p", portStr, "-join", joinStr}

	// If the host is not local, turn it into an ssh command line
	if randHost != hostname && randHost != "localhost" {
		sshLine := []string{"ssh", randHost}
		commandLine = append(sshLine, commandLine...)
	}

	newNode := randHost + portStr
	log.Printf("Starting %s: %s", newNode, commandLine)

	// Run the command
	cmd := exec.Command(commandLine[0], commandLine[1:]...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	err = cmd.Start()

	if err != nil {
		return "", err
	}
	return newNode, nil
}

func shutdownHandler(w http.ResponseWriter, r *http.Request) {

	// Only accept POST
	if r.Method != "POST" {
		http.Error(w, "Method not allowed", 405)
		return
	}

	// Send response
	body := "Shutting down\n"
	fmt.Fprintf(w, body)

	// Trigger shutdown
	exitReason <- "HTTP /shutdown request"
}

// Invokes an rpc call, sending a json formatted node that wants to join the network
func joinNetwork(joinPeer string) error {
	neighboursLock.Lock()
	marshaledgNode, _ := json.Marshal(gNode)
	response, _ := http.Post("http://"+joinPeer+"/joinnetwork", "json", bytes.NewReader(marshaledgNode))
	buf := readFromReadCloser(response.Body)
	var n NodeName
	json.Unmarshal(buf.Bytes(), &n)
	gNode.successor = n
	neighboursLock.Unlock()
	return nil
}

func neighboursHandler(w http.ResponseWriter, r *http.Request) {
	// Send response
	neighboursLock.RLock()
	fmt.Fprintln(w, gNode.successor.Name+"\n"+gNode.predecessor.Name)
	neighboursLock.RUnlock()
}

// Lets a node join the network by the node being joined on providing its successor
// as the new nodes successor, and the joined on node uses the new node as its successor
func joinNetworkHandler(w http.ResponseWriter, r *http.Request) {
	log.Println("Received JoinNetwork Request")
	b := readFromReadCloser(r.Body)
	log.Println(b.String())
	var joiningnode NodeName
	json.Unmarshal(b.Bytes(), &joiningnode)
	log.Println(joiningnode)
	ret, _ := json.Marshal(gNode.successor)
	w.Write(ret)
	gNode.successor = joiningnode
	// Find successor and return it to caller
}

//invokes RPC to successor to set successor.predecessor = gNode
func notify(nodetonotify NodeName) {
	marshaledgNode, _ := json.Marshal(gNode)
	http.Post("http://"+nodetonotify.Name+"/notify", "JSON", bytes.NewReader(marshaledgNode))
}

// Sets predecessor to node being notified by, except in certain cases
func notifyHandler(w http.ResponseWriter, r *http.Request) {
	b := readFromReadCloser(r.Body)
	log.Println(b.String())
	var notifyingnode NodeName
	json.Unmarshal(b.Bytes(), &notifyingnode)
	if gNode.predecessor.Name == "" || containedin(notifyingnode.ID, gNode.predecessor.ID, gNode.ID) {
		log.Println("setting new predecessor: ", notifyingnode)
		gNode.predecessor = notifyingnode
	}
}

// Writes back its own predecessor
func findPredecessorHandler(w http.ResponseWriter, r *http.Request) {
	buf, _ := json.Marshal(gNode.predecessor)
	w.Write(buf)
}

// Checks whether a nodes successor has it as predecessor
func stabilize() {
	if gNode.successor.Name == gNode.Name {
		return
	}
	log.Println("stabilize: successor is: ", gNode.successor.Name)
	msg, _ := json.Marshal(gNode.successor)
	response, _ := http.Post("http://"+gNode.successor.Name+"/findPredecessor", "JSON", bytes.NewReader(msg))
	log.Println("stabilize: attempting to read response..")
	b := readFromReadCloser(response.Body)
	var successorsPredecessor NodeName
	json.Unmarshal(b.Bytes(), &successorsPredecessor)
	log.Println(successorsPredecessor)
	if successorsPredecessor.Name != "" && containedin(successorsPredecessor.ID, gNode.ID, gNode.successor.ID) && (successorsPredecessor.Name != gNode.Name) {
		gNode.successor = successorsPredecessor
	}
	notify(gNode.successor)
}

// helper func
func containedin(x uint64, a uint64, b uint64) bool {
	var left, right uint64
	if a > b {
		if x < a || x > b {
			return true
		}
		return false
	} else {
		left = a
		right = b
	}
	if a == b {
		return true
	}
	if x > left && x < right {
		return true
	}
	return false
}

//helper func
func readFromReadCloser(r io.ReadCloser) *bytes.Buffer {
	buf := new(bytes.Buffer)
	buf.ReadFrom(r)
	return buf
}
