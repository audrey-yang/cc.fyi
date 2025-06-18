package main

import (
	"strconv"
	"strings"
)

type RESPType int

const (
	SimpleString RESPType = iota
	Error
	Integer
	BulkString
	Null
	Array
)

type message struct {
	respType RESPType
	value    any
}

func deserialize(input string) message {
	if input[0] == '+' {
		parts := strings.Split(input[1:], "\r\n")
		return message{SimpleString, parts[0]}
	}
	if input[0] == '-' {
		parts := strings.Split(input[1:], "\r\n")
		return message{Error, parts[0]}
	}
	if input[0] == ':' {
		parts := strings.Split(input[1:], "\r\n")
		num, _ := strconv.Atoi(parts[0])
		return message{Integer, num}
	}
	if input[0] == '$' {
		parts := strings.Split(input[1:], "\r\n")
		len, _ := strconv.Atoi(parts[0])
		if len == -1 {
			return message{Null, nil}
		}
		binString := make([]byte, len)
		for i := range len {
			binString[i] = byte(parts[1][i])
		}
		return message{BulkString, binString}
	}
	if input[0] == '*' {
		parts := strings.Split(input[1:], "\r\n")
		num, _ := strconv.Atoi(parts[0])
		arr := make([]any, num)
		j := 1
		for i := range num {
			var subInput string
			if parts[j][0] == '$' {
				subInput = parts[j] + "\r\n" + parts[j+1]
				j += 1
			} else {
				subInput = parts[j]
			}
			j += 1
			arr[i] = deserialize(subInput)
		}
		return message{Array, arr}
	}

	panic("Cannot deserialize input " + input)
}
