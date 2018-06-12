package main

import (
	"bytes"
	"compress/gzip"
	"crypto/sha1"
	"encoding/gob"
	"fmt"
	"io"
	"log"
	"net"
	"os"
	"sync"
	"time"
)

const CHUNK_SIZE = 512

var mutex = &sync.Mutex{}

/*
TODO DESC ABT THIS STRUCT
*/
type FileHeaders struct {
	Size    int64
	LenName int
	Name    string
}

type ChunkHeader struct {
	ChunkSize int
	FP        [20]byte
	Nr        int64
}

func main() {
	listener, err := net.Listen("tcp", "localhost:8000") // Listen for conn here
	if err != nil {
		log.Fatal(err)
	}
	// Establish connection
	c, err := listener.Accept()
	if err != nil {
		log.Fatal(err)
	}
	defer c.Close() // Don't close until func is finished

	fmt.Println("\nClient connected!")

	// File to sent
	file, err := os.Open("../test.jpg")
	if err != nil {
		fmt.Println(err)
		return
	}

	// Retrieve file information
	fileInfo, err := file.Stat()
	if err != nil {
		fmt.Println(err)
		return
	}
	enc := gob.NewEncoder(c)
	fpMap := make(map[[20]byte][20]byte) // Fingerprint map

	// Channels
	readChunk := make(chan []byte)
	readChunkSize := make(chan int)
	done := make(chan bool)
	pkgnum := make(chan int64)

	fmt.Println("\nStart sending file!")
	// Encode and send header info
	EncodeHeader(c, file, fileInfo, enc)
	go ReadChunk(c, readChunk, fileInfo, file, readChunkSize, pkgnum) // One thread for reading from file
	start := time.Now()
	for i := 0; i < 10; i++ {
		go SendChunk(readChunk, c, fileInfo, readChunkSize, enc, fpMap, done, pkgnum) // One thread for sending data
	}
	<-done
	end := time.Since(start)
	fmt.Println("Time used sending chunks: ", end)
}

/*
TODO DESCRIPTION ABT THIS FUNC
*/
func ReadChunk(c net.Conn, readChunk chan []byte, fileInfo os.FileInfo, file *os.File, readChunkSize chan int, pkgnum chan int64) {
	// New encoder for rest of chunks
	var i int64 = 0
	for {

		// Alloc buf of arbitrary size
		sendBuffer := make([]byte, CHUNK_SIZE)
		// chunkSize is how much is actually read
		chunkSize, err := file.Read(sendBuffer)
		if err == io.EOF {
			break
		}
		// update the channels
		readChunk <- sendBuffer
		readChunkSize <- chunkSize
		pkgnum <- i
		i++
	}
	close(readChunk)
	fmt.Println("\nFile has been sent, closing connection")
}

func SendChunk(readChunk chan []byte, c net.Conn, fileInfo os.FileInfo, rch chan int, enc *gob.Encoder, fpMap map[[20]byte][20]byte, done chan bool, pkgnum chan int64) {
	count := 0
	for data := range readChunk {

		ch := <-rch
		var newBuf bytes.Buffer
		fp := sha1.Sum(data)
		mutex.Lock()
		EncodeChunkHeader(c, enc, ch, fp, pkgnum)
		count++
		if fpMap[fp] != fp {
			count--
			fpMap[fp] = fp

			zip := gzip.NewWriter(&newBuf)
			zip.Write(data)
			zip.Close()
			enc.Encode(newBuf.Bytes())
		}
		mutex.Unlock()
	}
	fmt.Println("number of cache hits: ", count)
	done <- true
}

func EncodeHeader(c net.Conn, file *os.File, fileInfo os.FileInfo, enc *gob.Encoder) {
	// Create instance of struct and fill it

	f := FileHeaders{
		Size:    fileInfo.Size(),
		LenName: len(file.Name()),
		Name:    file.Name()}

	err := enc.Encode(FileHeaders{f.Size, f.LenName, f.Name})
	if err != nil {
		log.Fatal("\nEncode error:", err)
	}
}

func EncodeChunkHeader(c net.Conn, enc *gob.Encoder, chunkSize int, fp [20]byte, pkgnum chan int64) {
	h := ChunkHeader{
		ChunkSize: chunkSize,
		FP:        fp,
		Nr:        <-pkgnum}

	err := enc.Encode(ChunkHeader{h.ChunkSize, h.FP, h.Nr})
	if err != nil {
		log.Fatal("\nEncode error:", err)
	}
}
