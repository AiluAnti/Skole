#!/bin/sh

# Regenerate static assets

# $GOPATH/bin must be in your PATH for this to work.
# We go out of our way to use the direct path here,
# but the go-bindata-assetfs code that we call does not.

go get github.com/jteeuwen/go-bindata/...
go get github.com/elazarl/go-bindata-assetfs/...

$(go env GOPATH)/bin/go-bindata-assetfs static/...
