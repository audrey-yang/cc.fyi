package main

import (
	"fmt"
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
	return nil
}

func main() {
	fmt.Println(parse("+OK\r\n"))
	fmt.Println(parse("-Error message\r\n"))
}
