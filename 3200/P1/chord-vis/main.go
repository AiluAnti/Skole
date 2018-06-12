package main

import (
	"bytes"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"math/rand"
	"net/http"
	"os"
	"strings"
	"sync"
	"time"

	"github.com/gorilla/websocket"

	"../config"
)

// Static assets are in bindata_assetfs.go, which is generated from the files
// in the static directory.
// Run `go generate` to re-generate after a change

//go:generate ./generate.sh

var hostname string
var port string
var maxRunTime time.Duration
var exitReason chan string

var conf config.Config

var polling map[string]bool
var pollingLock sync.RWMutex
var pollInterval time.Duration
var waitForNodeStart time.Duration

// Messages to and from the client
type event struct {
	Command        string
	NodeName       string
	ConnectedNodes []string
}

var toClient chan event
var fromClient chan event
var addSocket chan *websocket.Conn

func main() {
	var startNodeName string

	// Get variables from environment
	hostname = config.Hostname()

	// Parse command line params
	flag.StringVar(&startNodeName, "join", hostname+":8181", "node to begin polling from")

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
	port = fmt.Sprintf(":%d", conf.VisPort)
	pollInterval, err = time.ParseDuration(conf.VisPollInterval)
	if err != nil {
		log.Fatalf("Invalid VisPollInterval duration in config: %s: %s", conf.VisPollInterval, err)
	}
	waitForNodeStart, err = time.ParseDuration(conf.VisWaitForNodeStart)
	if err != nil {
		log.Fatalf("Invalid VisWaitForNodeStart duration in config: %s: %s", conf.VisPollInterval, err)
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

	// Start poll loop
	polling = make(map[string]bool)
	toClient = make(chan event)
	fromClient = make(chan event)
	addSocket = make(chan *websocket.Conn)

	go pollLoop(startNodeName)
	go socketDispatchLoop()
	go fromClientReadLoop()

	// Set up http handler paths
	http.HandleFunc("/graphsocket", graphSocketHandler)
	http.Handle("/", http.FileServer(assetFS()))

	// Start server
	log.Printf("Starting node on %s%s\n", hostname, port)
	err = http.ListenAndServe(port, nil)

	if err != nil {
		log.Panic(err)
	}
}

func pollLoop(nodeName string) {

	// Mark the node as being polled, or abort if already being polled
	pollingLock.Lock()
	alreadyPolling := polling[nodeName]
	if !alreadyPolling {
		polling[nodeName] = true
	}
	pollingLock.Unlock()

	if alreadyPolling {
		return
	}

	// Give the new node a second to start up
	<-time.After(waitForNodeStart)

	log.Printf("Polling %s: Starting loop", nodeName)

	// Start a timer
	ticker := time.NewTicker(pollInterval)
	for range ticker.C {
		// Make the neighbours request
		neighbours, err := GETneighbours(nodeName)
		if err != nil {
			// Error polling node: send "remove" and stop polling
			log.Printf("Polling %s: %s", nodeName, err)
			toClient <- event{
				Command:  "remove",
				NodeName: nodeName,
			}
			break
		}

		// Launch a new poll loop for each new neighbour
		// The check at the top of the function will prevent duplicates
		for _, neighbour := range neighbours {
			go pollLoop(neighbour)
		}

		// Send an "update" event to the client
		toClient <- event{
			Command:        "update",
			NodeName:       nodeName,
			ConnectedNodes: neighbours,
		}
	}

	// Stop the timer
	ticker.Stop()

	// Unmark the node
	pollingLock.Lock()
	delete(polling, nodeName)
	pollingLock.Unlock()

	log.Printf("Polling %s: Stopped loop", nodeName)
}

func fromClientReadLoop() {

	for evt := range fromClient {

		switch evt.Command {
		case "addNode":
			newNode, err := POSTaddNode(evt.NodeName)
			if err != nil {
				log.Print(err)
				continue
			}

			// Begin polling the new node
			go pollLoop(newNode)
		}
	}
}

func graphSocketHandler(w http.ResponseWriter, r *http.Request) {

	handlerId := rand.Uint32()

	// Accept websocket connection
	log.Printf("%x: Starting websocket", handlerId)
	upgrader := websocket.Upgrader{}
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println(err)
		return
	}

	go socketReadLoop(conn)
	addSocket <- conn
}

func socketDispatchLoop() {

	sockets := []*websocket.Conn{}
	for {
		select {

		// New socket: start socket read loop and append to list
		case socket := <-addSocket:
			sockets = append(sockets, socket)

		// New outgoing event: send to all sockets
		case evt := <-toClient:
			for i := 0; i < len(sockets); {
				err := sockets[i].WriteJSON(evt)
				if err != nil {
					// Error writing to socket: remove from list
					log.Println(err)
					copy(sockets[i+1:], sockets[i:])
					sockets = sockets[0 : len(sockets)-1]
				} else {
					// No error: next socket
					i++
				}
			}
		}
	}
}

func socketReadLoop(socket *websocket.Conn) {
	for {
		// Read from socket
		incoming := event{}
		if err := socket.ReadJSON(&incoming); err != nil {
			log.Print(err)
			break
		}
		log.Print("Message from server: ", incoming)
		fromClient <- incoming
	}
}

// Interacting with the nodes

func GETneighbours(nodeName string) ([]string, error) {
	nodeUrl := fmt.Sprintf("http://%s/neighbours", nodeName)

	// Make the neighbours request
	resp, err := http.Get(nodeUrl)
	if err != nil {
		return []string{}, err
	}

	// Read and parse the response
	bytes, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return []string{}, err
	}
	str := strings.TrimSpace(string(bytes))
	neighbours := []string{}
	if str != "" {
		neighbours = strings.Split(str, "\n")
	}

	return neighbours, nil
}

func POSTaddNode(nodeName string) (string, error) {
	nodeUrl := fmt.Sprintf("http://%s/addNode", nodeName)
	// POST to addNode
	resp, err := http.Post(nodeUrl, "text/plain", bytes.NewBuffer([]byte{}))
	if err != nil {
		return "", err
	}
	// Get new node from response
	bytes, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return "", err
	}
	newNode := strings.TrimSpace(string(bytes))
	log.Printf("New node: %s", newNode)
	return newNode, nil
}
