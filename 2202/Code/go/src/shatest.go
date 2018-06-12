/*
To make this work when transferring files:
1. filesize is needed to calculate amount of blocks
2. Write each block to a buf
*/

package main

import (
	"crypto/sha1" // For å lage hash-obj + compute sha1-sum
	"fmt"
	// for å skrive til stdout
)

const Blocksize = 512
const Size = 160

func main() {
	hash := sha1.New()
	str := "Prell er best"

	hash.Write([]byte(str))
	fmt.Printf("%x", hash.Sum(nil))
}
