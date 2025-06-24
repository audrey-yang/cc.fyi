package main

import (
	"fmt"
	"net"
	"redis/internal/resp"
	"sync"
)

type Redis struct {
    mutex   sync.Mutex
    dict    map[string]string
}

var redis = Redis{sync.Mutex{}, make(map[string]string)}

func handleSet(cmds []any) resp.Message {
    var ret resp.Message
    keyMsg, _ := cmds[1].(resp.Message)
    key, _ := keyMsg.Value.([]byte)
    valMsg, _ := cmds[2].(resp.Message)
    val, _ := valMsg.Value.([]byte)

    if len(cmds) == 4 {
        val, exists := redis.dict[string(key)]
        if exists {
            ret = resp.Message{RespType: resp.BulkString, Value: []byte(val)}
        } else {
            ret = resp.Message{RespType: resp.Null, Value: nil}
        }
    } else {
        ret = resp.Message{RespType: resp.SimpleString, Value: "OK"}
    }
    redis.mutex.Lock()
    redis.dict[string(key)] = string(val)
    redis.mutex.Unlock()
    fmt.Printf("Set key:%s, val:%s\n", string(key), string(val))
    return ret
}

func handleGet(cmds []any) resp.Message {
    var ret resp.Message
    keyMsg, _ := cmds[1].(resp.Message)
    key, _ := keyMsg.Value.([]byte)
    val, exists := redis.dict[string(key)]
    if exists {
        ret = resp.Message{RespType: resp.BulkString, Value: []byte(val)}
        fmt.Printf("Got key:%s, val:%s\n", string(key), string(val))
    } else {
        ret = resp.Message{RespType: resp.Null, Value: nil}
        fmt.Printf("Failed to get key:%s\n", string(key))
    }
    return ret
}

func handleConnection(conn net.Conn) {
    defer conn.Close()

    buf := make([]byte, 1024)
    for {
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
                if len(cmds) == 1 {
                    pong := resp.Message{RespType: resp.BulkString, Value: []byte("PONG")}
                    ret = resp.Message{
                        RespType: resp.Array, 
                        Value: []any{pong},
                    }
                } else {
                    ret = resp.Message{
                        RespType: resp.Array, 
                        Value: cmds[1:],
                    }
                }
            } else if string(firstCmd) == "ECHO" {
                ret = resp.Message{RespType: resp.Array, Value: cmds[1:]}
            } else if string(firstCmd) == "SET" {
                ret = handleSet(cmds)
            } else if string(firstCmd) == "GET" {
                ret = handleGet(cmds)
            } else if string(firstCmd) == "HELLO" {
                pong := resp.Message{RespType: resp.BulkString, Value: []byte("PONG")}
                ret = resp.Message{
                    RespType: resp.Array, 
                    Value: []any{pong},
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
