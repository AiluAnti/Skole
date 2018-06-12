package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"math/rand"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"../config"
	"./chord"
)

var hostname string
var port string
var maxRunTime time.Duration
var exitReason chan string

var conf config.Config

var neighbours string
var neighboursLock sync.RWMutex

var node *chord.Node

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

	// Creating a new node object.
	node, err = chord.New(chord.PortIncrement(hostname + port))
	if err != nil {
		log.Fatal("Fatal: Could not create chord node")
	}

	if joinPeer != "" {
		err := joinNetwork(joinPeer)
		if err != nil {
			log.Fatal("Fatal: Could not join network: ", err)
		}
	} else {
		// Creating a new network
		node.Create()
		log.Print("No peers to join. New network")
	}

	// Set up http handler paths
	http.HandleFunc("/", indexHandler)
	http.HandleFunc("/addNode", addNodeHandler)
	http.HandleFunc("/shutdown", shutdownHandler)
	http.HandleFunc("/neighbours", neighboursHandler)
	http.HandleFunc("/storage/", distributedDataStorage)

	// Start server
	log.Printf("Starting node on %s%s\n", hostname, port)
	err = http.ListenAndServe(port, nil)

	if err != nil {
		log.Fatal(err)
	}
}

func distributedDataStorage(w http.ResponseWriter, r *http.Request) {
	key := r.RequestURI[strings.LastIndex(r.RequestURI, "/")+1:]
	if r.Method == "PUT" {
		value, err := ioutil.ReadAll(r.Body)
		if err != nil {
			return
		}
		err = node.Put(key, value)
		if err != nil {
			return
		}

	} else if r.Method == "GET" {
		value, err := node.Get(key)
		if err != nil {
			return
		}
		w.Write(*value)
	} else {
		http.Error(w, "Method not allowed", 405)
	}
}

func indexHandler(w http.ResponseWriter, r *http.Request) {

	// Send response
	body := "Node running on " + hostname + port + "\n"
	fmt.Fprintf(w, body)
}

/*
 *
 * TODO: Bootstrap a new node and return the ip:port pair of the new node.
 */
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

/*
 * /shutdown
 * TODO: Update the sucessor of the predecessor node.
 *
 * Node leave the network.
 */
func shutdownHandler(w http.ResponseWriter, r *http.Request) {

	// Only accept POST
	if r.Method != "POST" {
		http.Error(w, "Method not allowed", 405)
		return
	}
	node.Close()
	// Send response
	body := "Shutting down\n"
	fmt.Fprintf(w, body)

	// Trigger shutdown
	exitReason <- "HTTP /shutdown request"
}

func joinNetwork(joinPeer string) error {
	// TODO: Actually join the network ambiguity
	neighboursLock.Lock()
	// Joins network
	node.Join(chord.PortIncrement(joinPeer))
	neighboursLock.Unlock()
	return nil
}

/*
 * /neighbours
 * TODO: Should return a list of ip and port pairs of all nodes connected to the recipent node.
 *
 * For the chord implementation, this should be the sucessor(s) of the node. The response body
 * must be formatted as a list of ip:port (e.g 1237.0.0.1:1234) entries with newline separating
 * each ip:port pair.
 */
func neighboursHandler(w http.ResponseWriter, r *http.Request) {

	// Send response
	neighboursLock.RLock()
	fmt.Fprintln(w, strings.Join(node.GetN(), "\n"))
	neighboursLock.RUnlock()
}
