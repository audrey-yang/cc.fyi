package main

import (
	"fmt"
	"net"
	"redis/internal/resp"
)

var dict = make(map[string]string)

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

        message := resp.Deserialize(string(buf))

        if cmds, ok := message.Value.([]any); ok {
            firstCmdMsg, _ := cmds[0].(resp.Message)
            firstCmd, _ := firstCmdMsg.Value.([]byte)
            var ret resp.Message
            if string(firstCmd) == "PING" {
                pong := resp.Message{RespType: resp.BulkString, Value: []byte("PONG")}
                ret = resp.Message{
                    RespType: resp.Array, 
                    Value: []any{pong},
                }
            } else if string(firstCmd) == "ECHO" {
                ret = resp.Message{RespType: resp.Array, Value: cmds[1:]}
            } else if string(firstCmd) == "SET" {
                keyMsg, _ := cmds[1].(resp.Message)
                key, _ := keyMsg.Value.([]byte)
                valMsg, _ := cmds[2].(resp.Message)
                val, _ := valMsg.Value.([]byte)
                dict[string(key)] = string(val)
                ret = resp.Message{RespType: resp.SimpleString, Value: "OK"}
                fmt.Printf("Set key:%s, val:%s\n", string(key), dict[string(key)])
            } else if string(firstCmd) == "GET" {
                keyMsg, _ := cmds[1].(resp.Message)
                key, _ := keyMsg.Value.([]byte)
                val, exists := dict[string(key)]
                if exists {
                    ret = resp.Message{RespType: resp.BulkString, Value: []byte(val)}
                    fmt.Printf("Got key:%s, val:%s\n", string(key), string(val))
                } else {
                    ret = resp.Message{RespType: resp.Error, Value: "Not found"}
                    fmt.Printf("Failed to get key:%s\n", string(key))
                }
            } else {
                ret = resp.Message{RespType: resp.Error, Value: "ERROR"}
            }
            conn.Write([]byte(resp.Serialize(ret)))
        } else {
            fmt.Println("Something went wrong; couldn't convert")
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
