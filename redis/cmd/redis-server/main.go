package main

import (
	"fmt"
	"net"
	"strings"
)

var dict = make(map[string]any)

func handleConnection(conn net.Conn) {
    defer conn.Close()

    for {
		buf := make([]byte, 64)
		bytes, err := conn.Read(buf)
		if err != nil {
			fmt.Println(err)
			return
		}

		if bytes == 0 {
			return
		}

        cmds := strings.Split(string(buf[:bytes]), " ")
		fmt.Printf("Received: %v\n", cmds)
        if cmds[0] == "PING" {
            conn.Write([]byte("PONG"))
        } else if cmds[0] == "ECHO" {
            conn.Write(buf[5:])
        } else if cmds[0] == "SET" {
            dict[cmds[1]] = cmds[2]
            conn.Write([]byte("OK"))
        }  else if cmds[0] == "GET" {
            if val, ok := dict[cmds[1]].(string); ok {
                conn.Write([]byte(val))
            }
        }
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
