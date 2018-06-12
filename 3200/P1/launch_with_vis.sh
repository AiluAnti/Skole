#!/bin/bash

print_usage() {
cat <<-EOF
    $0 - Launch node, launch visualizer, and cleanup

    Usage: $0 NODE_HOST NODE_PORT

    Other settings will be in the configuration JSON file.
EOF
}

# Check arguments
if [[ $# -eq 0 ]]
then
    print_usage
    exit 1
fi

NODE_HOST=$1
NODE_PORT=$2


# Launch on given host. Only use SSH if not local.
launch() {
    HOST=$1
    shift
    CMD=$*

    if [[ $HOST != $(hostname) && $HOST != "localhost" ]]
    then
        set -x
        ssh $HOST $CMD
        set +x
    else
        set -x
        $CMD
        set +x
    fi
}

# Start node
launch $NODE_HOST $PWD/chord-node/chord-node -p ":$NODE_PORT" &

# Start the visualizer
$PWD/chord-vis/chord-vis -join "$NODE_HOST:$NODE_PORT" &

# Wait for enter
read -p "==== Press enter to quit ==="

# Clean up
if [[ -n $(which rocks) ]]
then
    ./kill_compute.sh
fi

killall chord-vis chord-node
