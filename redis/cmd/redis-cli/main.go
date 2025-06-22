package main

import (
	"fmt"
	"net"
	"os"
	"strings"
)

func main() {
    conn, err := net.Dial("tcp", "localhost:6379")
    if err != nil {
        fmt.Println(err)
        return
    }

    _, err = conn.Write([]byte(strings.Join(os.Args[1:], " ")))
    if err != nil {
        fmt.Println(err)
        return
    }

    buf := make([]byte, 1024)
    bytes, err2 := conn.Read(buf)
    if err2 != nil {
        fmt.Println(err2)
        return
    }

    fmt.Printf("Received: %s", string(buf[:bytes+1]))

    conn.Close()
}

