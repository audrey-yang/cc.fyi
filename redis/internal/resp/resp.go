package resp

import (
	"strconv"
	"strings"
)

type RespType int

const (
	SimpleString RespType = iota
	Error
	Integer
	BulkString
	Null
	Array
)

type Message struct {
	RespType RespType
	Value    any
}

func Deserialize(input string) Message {
	if input[0] == '+' {
		parts := strings.Split(input[1:], "\r\n")
		return Message{SimpleString, parts[0]}
	}
	if input[0] == '-' {
		parts := strings.Split(input[1:], "\r\n")
		return Message{Error, parts[0]}
	}
	if input[0] == ':' {
		parts := strings.Split(input[1:], "\r\n")
		num, _ := strconv.Atoi(parts[0])
		return Message{Integer, num}
	}
	if input[0] == '$' {
		parts := strings.Split(input[1:], "\r\n")
		len, _ := strconv.Atoi(parts[0])
		if len == -1 {
			return Message{Null, nil}
		}
		binString := make([]byte, len)
		for i := range len {
			binString[i] = byte(parts[1][i])
		}
		return Message{BulkString, binString}
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
		return Message{Array, arr}
	}

	panic("Cannot deserialize input " + input)
}

func Serialize(input Message) string {
	switch input.RespType {
	case SimpleString:
		return "+" + input.Value.(string) + "\r\n"
	case Error:
		return "-" + input.Value.(string) + "\r\n"
	case Integer:
		return ":" + input.Value.(string) + "\r\n"
	case BulkString:
		bulkString := input.Value.([]byte)
		return "$" + strconv.Itoa(len(bulkString)) + "\r\n" + string(bulkString) + "\r\n"
	case Null:
		return "$-1\r\n"
	case Array:
		arr := input.Value.([]any)
		serialized := "*" + strconv.Itoa(len(arr)) + "\r\n"
		for _, v := range arr {
			v, _ := v.(Message)
			serialized += Serialize(v)
		}
		return serialized
	}
	panic("Cannot serialize input")
}

func strToBytes(s string) Message {
	bytes := make([]byte, len(s))
	for i, c := range s {
		bytes[i] = byte(c)
	}
	return Message{BulkString, bytes}
}

func CreateMessage(args []string) Message {
	bulkStrings := make([]any, len(args))
	for i, arg := range args {
		bulkStrings[i] = strToBytes(arg)
	}
	return Message{
		RespType: Array,
		Value: bulkStrings,
	}
}