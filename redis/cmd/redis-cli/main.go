package main

import (
	"fmt"
	"net"
)

func main() {
    conn, err := net.Dial("tcp", "localhost:6379")
    if err != nil {
        fmt.Println(err)
        return
    }

    _, err = conn.Write([]byte("PING\r\n"))
    if err != nil {
        fmt.Println(err)
        return
    }

    conn.Close()
}

