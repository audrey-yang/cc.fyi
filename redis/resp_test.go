package main

import (
	"testing"
)

func strToBytes(s string) message {
	bytes := make([]byte, len(s))
	for i, c := range s {
		bytes[i] = byte(c)
	}
	return message{BulkString, bytes}
}

func compare(result message, expected message) bool {
	if result.respType != expected.respType {
		return false
	}
	if result.respType == Array {
		for i, v := range result.value.([]any) {
			v, vok := v.(message)
			e, eok := expected.value.([]message)
			if !vok || !eok || !compare(v, e[i]) {
				return false
			}
		}
	} else if result.respType == BulkString {
		for i, v := range result.value.([]byte) {
			e, eok := expected.value.([]byte)
			if !eok || v != e[i] {
				return false
			}
		}
	} else {
		if result.value != expected.value {
			return false
		}
	}
	return true
}

func TestDeserialize(t *testing.T) {
	tests := []struct {
		input    string
		expected message
	}{
		{"$-1\r\n", message{Null, nil}},
		{"*1\r\n$4\r\nping\r\n", message{Array, []message{strToBytes("ping")}}},
		{"*2\r\n$4\r\necho\r\n$11\r\nhello world\r\n",
			message{Array, []message{strToBytes("echo"), strToBytes("hello world")}}},
		{"*3\r\n$3\r\nget\r\n$3\r\nkey\r\n:255\r\n",
			message{Array, []message{strToBytes("get"), strToBytes("key"), message{Integer, 255}}}},
		{"+OK\r\n", message{SimpleString, "OK"}},
		{"+hello world\r\n", message{SimpleString, "hello world"}},
		{"-Error message\r\n", message{Error, "Error message"}},
		{":2\r\n", message{Integer, 2}},
		{"$0\r\n\r\n", message{BulkString, strToBytes("")}},
	}

	for _, test := range tests {
		testname := test.input
		t.Run(testname, func(t *testing.T) {
			result := deserialize(test.input)
			if !compare(result, test.expected) {
				t.Errorf("Expected %v, but got %v", test.expected, result)
			}
		})
	}
}
