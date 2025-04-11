package main

import (
	"bufio"
	"fmt"
	"os"
)

func main() {
	fileName := os.Args[1]
	f, err := os.Open(fileName)
	if err != nil {
		panic(err)
	}
	defer f.Close()

	scanner := bufio.NewScanner(f)
	scanner.Split(bufio.ScanWords)

	// var data map[string]interface{}
	valid := false
	for scanner.Scan() {
		t := scanner.Text()
		if t == "{}" {
			valid = true
			continue
		}
		fmt.Println(t)
	}

	if valid {
		fmt.Println("Valid JSON")
		os.Exit(0)
	}
	fmt.Println("Invalid JSON")
	os.Exit(1)
}
