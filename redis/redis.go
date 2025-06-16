package main

import (
	"fmt"
	"strconv"
	"strings"
)

func parse(input string) any {
	if input[0] == '+' {
		parts := strings.Split(input[1:], "\r\n")
		return parts[0]
	}
	if input[0] == '-' {
		parts := strings.Split(input[1:], "\r\n")
		return parts[0]
	}
	if input[0] == ':' {
		parts := strings.Split(input[1:], "\r\n")
		num, _ := strconv.Atoi(parts[0])
		return num
	}
	if input[0] == '$' {
		parts := strings.Split(input[1:], "\r\n")
		len, _ := strconv.Atoi(parts[0])
		if len == -1 {
			return nil
		}
		binString := make([]byte, len)
		for i := range len {
			binString[i] = byte(parts[1][i])
		}
		return binString
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
			arr[i] = parse(subInput)
		}
		return arr
	}
	return nil
}

func main() {
	fmt.Println(parse("$-1\r\n"))
	fmt.Println(parse("*1\r\n$4\r\nping\r\n"))
	fmt.Println(parse("*2\r\n$4\r\necho\r\n$11\r\nhello world\r\n"))
	fmt.Println(parse("*3\r\n$3\r\nget\r\n$3\r\nkey\r\n:255\r\n"))
	fmt.Println(parse("+OK\r\n"))
	fmt.Println(parse("-Error message\r\n"))
	fmt.Println(parse(":2\r\n"))
	fmt.Println(parse("$0\r\n\r\n"))
	fmt.Println(parse("+hello world\r\n"))
}
