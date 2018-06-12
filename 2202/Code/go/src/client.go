package main

import (
	"bytes"
	"compress/gzip"
	"encoding/gob"
	"fmt"
	"log"
	"net"
	"os"
	"sync"
	"time"
)

const CHUNK_SIZE = 512

var mutex = &sync.Mutex{}

type FileHeaders struct {
	Size    int
	LenName int
	Name    string
}

type ChunkHeader struct {
	ChunkSize int
	FP        [20]byte
	Nr        int64
}

func main() {
	c, err := net.Dial("tcp", "localhost:8000") // Establish connection
	if err != nil {
		log.Fatal(err)
	}

	// Don't close file until we are done using it
	defer c.Close()
	fmt.Println("\nConnected to server!")

	// Create new file on client side
	newFile, err := os.Create("test.jpg")
	if err != nil {
		panic(err)
	}

	dec := gob.NewDecoder(c)
	var ch ChunkHeader
	fpMap := make(map[[20]byte][]byte)

	unzipThis := make(chan []byte)
	done := make(chan bool)
	chunkHeader := make(chan ChunkHeader)

	f := DecodeHeader(c, dec)
	go DecodeChunkHeader(dec, f, ch, fpMap, newFile, unzipThis, chunkHeader)
	start := time.Now()
	go Unzip(unzipThis, fpMap, ch, newFile, done, chunkHeader)
	<-done
	end := time.Since(start)
	fmt.Println("Time elapsed: ", end)
}

func DecodeHeader(c net.Conn, dec *gob.Decoder) (f FileHeaders) {
	// Read in the struct
	err := dec.Decode(&f)
	if err != nil {
		log.Fatal("\ndecode error: ", err)
	}
	return f
}

// Decodes from decoder into receiveBuffer
func DecodeChunkHeader(dec *gob.Decoder, f FileHeaders, ch ChunkHeader, fpMap map[[20]byte][]byte, newFile *os.File, unzipThis chan []byte, chunkHeader chan ChunkHeader) {
	var i int
	// Retrieve data from stream and decode it
	for i = 0; i < f.Size; {
		// Decode header into ch
		err := dec.Decode(&ch)
		if err != nil {
			log.Fatal("\nDecode error: ", err)
			break
		}
		// Ready to decode the chunk
		DecodeChunk(fpMap, newFile, unzipThis, dec, ch, chunkHeader)
		i += ch.ChunkSize
	}
	close(unzipThis)
}

func DecodeChunk(fpMap map[[20]byte][]byte, newFile *os.File, unzipThis chan []byte, dec *gob.Decoder, ch ChunkHeader, chunkHeader chan ChunkHeader) {
	recvBuf := make([]byte, ch.ChunkSize)
	// FP does not currently exist
	mutex.Lock()
	if len(fpMap[ch.FP]) == 0 {
		err := dec.Decode(&recvBuf)
		mutex.Unlock()
		if err != nil {
			log.Fatal("\nDecode error:", err)
		}

		if len(recvBuf) == 0 {
			log.Fatal("\nReceivebuffer is empty")
			return
		}
		unzipThis <- recvBuf
		chunkHeader <- ch
		return

	} else {
		something := fpMap[ch.FP]
		newFile.WriteAt(something, ch.Nr*CHUNK_SIZE)
		mutex.Unlock()
		return
	}
}

// Unzips from receiveBuffer into something
func Unzip(unzipThis chan []byte, fpMap map[[20]byte][]byte, ch ChunkHeader, newFile *os.File, done chan bool, chunkHeader chan ChunkHeader) {
	for buf := range unzipThis {
		ch := <-chunkHeader
		something := make([]byte, ch.ChunkSize)
		reader := bytes.NewReader(buf)
		unz, err := gzip.NewReader(reader)
		if err != nil {
			log.Fatal("\nUnzip error: ", err)
		}
		mutex.Lock()
		unz.Read(something)
		unz.Close()
		fpMap[ch.FP] = something
		newFile.WriteAt(something, CHUNK_SIZE*ch.Nr)
		mutex.Unlock()
	}
	newFile.Close()
	done <- true
	fmt.Println("\nReceived file completely!")
}
