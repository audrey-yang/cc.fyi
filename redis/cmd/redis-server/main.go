package main

import (
	"fmt"
	"net"
	"redis/internal/resp"
)

func handleConnection(conn net.Conn) {
    defer conn.Close()

    for {
		buf := make([]byte, 1024)
		len, err := conn.Read(buf)
		if err != nil {
			fmt.Println(err)
			return
		}
		if len == 0 {
			return
		}
		fmt.Printf("Received: %v", resp.Deserialize(string(buf)))
	}
}

func main() {
    listener, err := net.Listen("tcp", ":6379")
    if err != nil {
        fmt.Println(err)
        return
    }

    fmt.Println("Hello! Listening on port 6379...")

    for {
        conn, err := listener.Accept()
        if err != nil {
            fmt.Println(err)
            continue
        }

        go handleConnection(conn)
    }
}
