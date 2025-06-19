package resp

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

func Deserialize(input string) message {
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
			arr[i] = Deserialize(subInput)
		}
		return message{Array, arr}
	}

	panic("Cannot deserialize input " + input)
}

func Serialize(input message) string {
	switch input.respType {
	case SimpleString:
		return "+" + input.value.(string) + "\r\n"
	case Error:
		return "-" + input.value.(string) + "\r\n"
	case Integer:
		return ":" + input.value.(string) + "\r\n"
	case BulkString:
		bulkString := input.value.([]byte)
		return "$" + strconv.Itoa(len(bulkString)) + "\r\n" + string(bulkString) + "\r\n"
	case Null:
		return "$-1\r\n"
	case Array:
		arr := input.value.([]any)
		serialized := "*" + strconv.Itoa(len(arr)) + "\r\n"
		for _, v := range arr {
			v, _ := v.(message)
			serialized += Serialize(v)
		}
		return serialized
	}
	panic("Cannot serialize input")
}

func strToBytes(s string) message {
	bytes := make([]byte, len(s))
	for i, c := range s {
		bytes[i] = byte(c)
	}
	return message{BulkString, bytes}
}

func CreateMessage(args []string) message {
	bulkStrings := make([]any, len(args))
	for i, arg := range args {
		bulkStrings[i] = strToBytes(arg)
	}
	return message{
		respType: Array,
		value: bulkStrings,
	}
}