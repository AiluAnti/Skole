Assignment 1, Chord: Starter Code
==================================================

This is the starter code for Assignment 1 in Distributed Systems Fundamentals
(INF-3200), fall semester 2017.

It consists of a skeleton node implementation, a utility to visualize your
network, and a few helper scripts.

The starter node will respond to the required API requests, and even spawn other
nodes. However, the network will be unstructured. The new node's only neighbour
will simply be the node that spawned it. Your task is to give the network
structure.

This starter code is written in Go (<http://golang.org/>), but you do not have
to use Go. You can do your implementation in any language that you can get
running on the cluster. If you are more comfortable with Python, look at
`chod-node-py/node.py` for an alternate skeleton implementation.

Code:

- `chord-node/`: The skeleton chord node
- `chord-node-py/`: Alternative skeleton chord node in Python
- `chord-vis/`: The visualizer utility
- `config/`: A simple library for reading configuration files

Helper scripts:

- `build-all.sh`: Builds both the node and the visualizer
- `launch_with_vis.sh`: Launches one node and the visualizer, then waits for you
  to press enter, then calls the `kill_compute.sh` script to clean up.
- `kill_compute.sh`: Kills all your processes on all compute notes

Generated config files:

- `chord-config.json`: Configure ports and compute nodes


Getting Started
==================================================

Setting up a Go environment on the cluster
--------------------------------------------------

To use Go, you will need to add the following lines to your `.bashrc` file on
the cluster:

    export GOROOT=/share/apps/go
    export PATH=$PATH:$GOROOT/bin

    export GOPATH=$HOME/go
    export PATH=$GOPATH/bin:$PATH


Working on your own machine
--------------------------------------------------

This code will also run on a single machine. It will just use multiple ports for
different nodes. You are welcome to do your development on your own machine, for
a more comfortable edit/rebuild/test cycle.

Just remember that your code will have to run on the cluster as well to pass the
assignment.


Fetching the code
--------------------------------------------------

Go's build system treats code differently, depending on whether it's inside or
outside your Go workspace (`~/go/src`). We developed this code outside of the
workspace, so it needs to be built there.

Find a place **outside** your Go workspace and clone the repository.

    git clone git@source.uit.no:inf3200/a1-chord.git
    cd a1-chord


Building
--------------------------------------------------

The `build-all.sh` script should take care of everything:

    ./build-all.sh

Look in that file for details of building each component.


Launching
--------------------------------------------------

The `launch_with_vis.sh` script will launch one node and the visualizer. It
takes two arguments, the hostname and port for the first node to launch.

    ./launch_with_vis.sh compute-1-1 9000

The visualizer will start its default port: 8182. If you are the only person
trying to run the script, you should be able to see the visualization at:

- <http://uvrocks.cs.uit.no:8182> -- if running on the cluster, or
- <http://localhost:8182> -- if running on your own machine

With the visualization, you can click on a node to have it spawn a new one.

When you're done, go back to the terminal window where the script is running and
press Enter. It will then call a cleanup script to terminate all nodes.

Note: If you try to run this script at the same time as someone else, you will
get a port conflict. You will need to change the port number and try again. You
change the port number by editing the generated config file.


Configuring: CHANGE THESE PORTS
--------------------------------------------------

After starting a node or the visualizer for the first time, it will write a
config file: `chord-config.json`. The file will look like this:

```json
{
    "VisPort": 8182,
    "VisPollInterval": "200ms",
    "VisWaitForNodeStart": "1000ms",
    "NodeLowPort": 9000,
    "NodeHighPort": 9999,
    "Hosts": [
        "compute-1-0",
        "compute-1-1",
        "compute-1-2",
        "compute-1-3",
    ],
    "MaxRun": "20m"
}
```

The fields are:

- `VisPort`: The port the visualizer should run on

- `VisPollInterval`: How often the visualizer should poll each node

- `VisWaitForNodeStart`: How long the visualizer should wait before starting to
  poll a new node

- `NodeLowPort`, `NodeHighPort`: Port range for nodes. When spawning a new node,
  the existing node will pick a random port in the range [NodeLowPort,
  NodeHighPort], inclusive

- `Hosts`: Hosts to run nodes on. When spawning a new node, the existing node
  will pick a random host from this list

- `MaxRun`: To guard against students forgetting to shut down, the nodes and the
  visualizer will automatically shut down after this amount of time

**YOU MUST CHANGE THESE PORTS** so that you do not conflict with other students.
If your node or visualizer does a panic saying "address already in use," that's
what happened. You have a conflict, and a server is already running on that
port. So pick new port numbers.

Remember that a port number is an unsigned 16-bit integer, so 0-65535. The port
range 49152-65535 is specifically for [ephemeral ports](
https://en.wikipedia.org/wiki/Ephemeral_port) like this, so that's a good range
to pick from.

The duration fields understand units like "ms", "s", and "m". They are parsed
with Go's standard time.ParseDuration function. See the [ParseDuration
documentation]( https://golang.org/pkg/time/#ParseDuration) for details.


Running Components Individually
==================================================

Building and running nodes manually
--------------------------------------------------

To rebuild the node code, simply run `go build` in the `chord-node/` directory.

    cd chord-node/
    go build

The executable takes two arguments `-p` for the port, and an optional `-join`
for an existing node to join. Note that here a colon is required in front of the
port.

    cd chord-node
    ./chord-node -p :8183
    ./chord-node -p :8184 -join $(hostname):8183

To launch on a compute node, use ssh:

    cd chord-node
    ssh compute-1-1 $PWD/chord-node -p :8183
    ssh compute-1-2 $PWD/chord-node -p :8183 -join compute-1-1:8183


Building and running the visualizer manually
--------------------------------------------------

To rebuild the visualizer, run `go build` in the `chord-vis/` directory:

    cd chord-vis
    go build

The HTML and JS files in `chord-vis/static/` are embedded in generated Go code
and built into the executable. If you change those files, update the generated
sources with `go generate`, then build.

    cd chord-vis
    go generate
    go build

To launch the visualizer, give it a node to start querying:

    ./chord-vis -join compute-1-1:8183


Talking to a running node with curl
--------------------------------------------------

The tool `curl` is essential when testing networks. It can understand and speak
just about every network protocol there is. And it excels at HTTP.

Get neighbours:

    curl http://compute-1-1:8183/neighours

To send an add node message:

    curl -X POST http://compute-1-1:8183/addNode

To send a shutdown message:

    curl -X POST http://compute-1-1:8183/shutdown


Development
==================================================

Node code
--------------------------------------------------

The code for the node is in `chord-node/main.go`. It should be relatively
straightforward. The add node, neighbours, and shutdown commands are handled by
the `addNodeHandler`, `neighboursHandler`, and `shutdownHandler` functions.

The function `joinNetwork` is called at startup, and it dictates how to join the
network. Right now all it does is add the node from the `-join` argument to the
list of neighbours. This is the core of your task. Get these nodes talking to
each other and giving the network a structure.

```go
func joinNetwork(joinPeer string) error {
	// TODO: Actually join the network
	neighboursLock.Lock()
	neighbours = append(neighbours, joinPeer)
	neighboursLock.Unlock()
	return nil
}
```


Visualizer code
--------------------------------------------------

The visualizer code is less straightforward. But you should not have to change
it, unless you get very ambitious.

The visualizer is a Go program that polls all the nodes in your network. For
each node, it will use the `neighbours/` API endpoint to probe the network graph
and to find more nodes.

It is also a web server. It serves up the HTML and JavaScript in the `static/`
subdirectory. That JavaScript uses the [vis.js]( http://visjs.org/) library to
draw a graph of your network. The JavaScript also opens a websocket connection
back to the server, and the server sends updates through the websocket as it
polls the nodes in the network.
