package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
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
				valid = false
				break
			}
		} else if keepScanning && t[len(t)-1] == ':' {
			maybeKey, err := strconv.Unquote(t[:len(t)-1])
			if err != nil {
				valid = false
				break
			}
			key = maybeKey
		} else if t[0] == '{' && t[len(t)-1] == ':' {
			maybeKey, err := strconv.Unquote(t[1 : len(t)-1])
			if err != nil {
				valid = false
				break
			}
			key = maybeKey
		} else {
			if key == "" {
				valid = false
				break
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

			intVal, err := strconv.Atoi(t)
			if err == nil {
				data[key] = intVal
				continue
			}

			if t == "true" {
				data[key] = true
				continue
			}

			if t == "false" {
				data[key] = false
				continue
			}

			if t == "null" {
				data[key] = nil
				continue
			}

			strVal, err := strconv.Unquote(t)
			if err == nil {
				data[key] = strVal
				continue
			}

			valid = false
			break
		}
	}

	if keepScanning {
		valid = false
	}

	if valid {
		fmt.Println("Valid JSON")
		fmt.Println(data)
		os.Exit(0)
	}
	fmt.Println("Invalid JSON")
	os.Exit(1)
}
