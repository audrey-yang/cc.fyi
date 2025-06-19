package main

import (
	"fmt"
	"net"
	"os"
	"redis/internal/resp"
)

func main() {
    conn, err := net.Dial("tcp", "localhost:6379")
    if err != nil {
        fmt.Println(err)
        return
    }

    args := os.Args
    message := resp.CreateMessage(args[1:])
    serializedMessage := resp.Serialize(message)
    fmt.Println(serializedMessage)

    _, err = conn.Write([]byte(serializedMessage))
    if err != nil {
        fmt.Println(err)
        return
    }

    conn.Close()
}

