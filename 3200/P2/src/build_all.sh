#!/bin/bash

set -e

(
    cd chord-node
    go build
    go fmt
)
(
    cd chord-vis
    go get github.com/gorilla/websocket
    go get github.com/jteeuwen/go-bindata/...
    go get github.com/elazarl/go-bindata-assetfs/...
    #go generate
    go build
    go fmt
)
(
    cd config/
    go fmt
)
