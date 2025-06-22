package resp

import (
	"testing"
)

func compareMessages(result Message, expected Message) bool {
	if result.RespType != expected.RespType {
		return false
	}
	if result.RespType == Array {
		for i, v := range result.Value.([]any) {
			v, vok := v.(Message)
			e, eok := expected.Value.([]Message)
			if !vok || !eok || !compareMessages(v, e[i]) {
				return false
			}
		}
	} else if result.RespType == BulkString {
		for i, v := range result.Value.([]byte) {
			e, eok := expected.Value.([]byte)
			if !eok || v != e[i] {
				return false
			}
		}
	} else {
		if result.Value != expected.Value {
			return false
		}
	}
	return true
}

func TestDeserialize(t *testing.T) {
	tests := []struct {
		input    string
		expected Message
	}{
		{"$-1\r\n", Message{Null, nil}},
		{"*1\r\n$4\r\nping\r\n", Message{Array, []Message{strToBytes("ping")}}},
		{"*2\r\n$4\r\necho\r\n$11\r\nhello world\r\n",
			Message{Array, []Message{strToBytes("echo"), strToBytes("hello world")}}},
		{"*3\r\n$3\r\nget\r\n$3\r\nkey\r\n:255\r\n",
			Message{Array, []Message{strToBytes("get"), strToBytes("key"), Message{Integer, 255}}}},
		{"+OK\r\n", Message{SimpleString, "OK"}},
		{"+hello world\r\n", Message{SimpleString, "hello world"}},
		{"-Error Message\r\n", Message{Error, "Error Message"}},
		{":2\r\n", Message{Integer, 2}},
		{"$0\r\n\r\n", Message{BulkString, strToBytes("")}},
	}

	for _, test := range tests {
		testname := test.input
		t.Run(testname, func(t *testing.T) {
			result := Deserialize(test.input)
			if !compareMessages(result, test.expected) {
				t.Errorf("Expected %v, but got %v", test.expected, result)
			}
		})
	}
}
