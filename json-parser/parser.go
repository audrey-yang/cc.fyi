package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
)

func tryParse(dataPtr *map[string]interface{}, keyPtr *string, tPtr *string) bool {
	data := *dataPtr
	key := *keyPtr
	t := *tPtr

	// Try to parse as int
	intVal, err := strconv.Atoi(t)
	if err == nil {
		data[key] = intVal
		return true
	}

	// Try to parse as bool
	if t == "true" {
		data[key] = true
		return true
	}

	if t == "false" {
		data[key] = false
		return true
	}

	// Try to parse null
	if t == "null" {
		data[key] = nil
		return true
	}

	// Try to parse as string
	strVal, err := strconv.Unquote(t)
	if err == nil {
		data[key] = strVal
		return true
	}

	fmt.Println("Error parsing value:", t, "for key", key)
	os.Exit(1)
	return false
}

func main() {
	fileName := os.Args[1]
	f, err := os.Open(fileName)
	if err != nil {
		panic(err)
	}
	defer f.Close()

	scanner := bufio.NewScanner(f)
	scanner.Split(bufio.ScanWords)

	var data map[string]interface{} = make(map[string]interface{})
	key := ""
	valid := false
	keepScanning := false

	for scanner.Scan() {
		t := scanner.Text()

		if t == "{}" {
			valid = true
			continue
		}

		valid = true

		if t == "{" {
			keepScanning = true
		} else if t == "}" {
			if keepScanning {
				fmt.Println("Error: Unmatched closing brace")
				os.Exit(1)
			}
		} else if keepScanning && t[len(t)-1] == ':' {
			maybeKey, err := strconv.Unquote(t[:len(t)-1])
			if err != nil {
				fmt.Println("Error parsing key:", t)
				os.Exit(1)
			}
			key = maybeKey
		} else if t[0] == '{' && t[len(t)-1] == ':' {
			maybeKey, err := strconv.Unquote(t[1 : len(t)-1])
			if err != nil {
				fmt.Println("Error parsing key:", t)
				os.Exit(1)
			}
			key = maybeKey
		} else {
			if key == "" {
				fmt.Println("Error: Key not found for value:", t)
				os.Exit(1)
			}
			if t[len(t)-1] == ',' {
				keepScanning = true
				t = t[:len(t)-1]
			} else if t[len(t)-1] == '}' {
				keepScanning = false
				t = t[:len(t)-1]
			} else {
				keepScanning = false
			}

			valid = tryParse(&data, &key, &t)
		}
	}

	if keepScanning {
		valid = false
		os.Exit(1)
	}

	if valid {
		fmt.Println("Parsed JSON:", data)
		os.Exit(0)
	}

	fmt.Println("Error: empty file")
	os.Exit(1)
}
