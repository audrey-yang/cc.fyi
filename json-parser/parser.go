package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
)

func tryParseValue(t string) interface{} {
	// Try to parse as int
	intVal, err := strconv.Atoi(t)
	if err == nil {
		return intVal
	}

	// Try to parse as bool
	if t == "true" {
		return true
	}

	if t == "false" {
		return false
	}

	// Try to parse null
	if t == "null" {
		return nil
	}

	// Try to parse as string
	strVal, err := strconv.Unquote(t)
	if err == nil {
		return strVal
	}

	fmt.Println("Error parsing value:", t)
	os.Exit(1)
	return false
}

func tryParseList(scanner *bufio.Scanner) (bool, *[]interface{}) {
	var data []interface{} = make([]interface{}, 0)

	keepScanningAfter := false
	for {
		t := scanner.Text()
		if t == "[]" || t == "[]," {
			return len(t) == 3, &data
		}

		if t == "]" || t == "]," {
			return len(t) == 3, &data
		}

		shouldReturn := false
		if t[len(t)-1] == ']' {
			t = t[:len(t)-1]
			shouldReturn = true
		} else if t[len(t)-2] == ']' {
			t = t[:len(t)-2]
			shouldReturn = true
			keepScanningAfter = true
		} else if t[len(t)-1] == ',' {
			t = t[:len(t)-1]
		}

		val := tryParseValue(t)
		data = append(data, val)

		res := scanner.Scan()
		if !res {
			fmt.Println("63Error: Unexpected end of file")
			os.Exit(1)
		}

		if shouldReturn {
			break
		}
	}
	return keepScanningAfter, &data
}

func tryParseObject(scanner *bufio.Scanner) (bool, *map[string]interface{}) {
	var data map[string]interface{} = make(map[string]interface{})

	keepScanning := false
	key := ""

	for {
		t := scanner.Text()
		// fmt.Println("/", t, "/")

		// Find key
		if len(key) == 0 {
			if t == "{}" || t == "{}," {
				return len(t) == 3, &data
			}

			if t == "{" {
				keepScanning = true
			} else if t[0] == '}' {
				if keepScanning {
					fmt.Println("88Error: Unexpected closing brace")
					os.Exit(1)
				}
				return len(t) == 2, &data
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
			}
		} else {
			if t[0] == '[' {
				var listPtr *[]interface{}
				keepScanning, listPtr = tryParseList(scanner)
				data[key] = *listPtr
				key = ""
				if keepScanning {
					scanner.Scan()
				}
				continue
			}
			if t[0] == '{' {
				var objPtr *map[string]interface{}
				keepScanning, objPtr = tryParseObject(scanner)
				data[key] = *objPtr
				key = ""
				if keepScanning {
					scanner.Scan()
				}
				continue
			}
			if t[len(t)-1] == ',' {
				keepScanning = true
				t = t[:len(t)-1]
			} else if t[len(t)-1] == '}' {
				t = t[:len(t)-1]
				val := tryParseValue(t)
				data[key] = val
				return false, &data
			} else if t[len(t)-2] == '}' && t[len(t)-1] == ',' {
				t = t[:len(t)-2]
				val := tryParseValue(t)
				data[key] = val
				return true, &data
			} else {
				keepScanning = false
			}

			val := tryParseValue(t)
			data[key] = val
			key = ""
		}

		res := scanner.Scan()
		if !res {
			fmt.Println("135Error: Unexpected end of file")
			os.Exit(1)
		}
	}
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
	res := scanner.Scan()
	if !res {
		fmt.Println("153Error: Unexpected end of file")
		os.Exit(1)
	}

	keepScanning, dataPtr := tryParseObject(scanner)
	data := *dataPtr
	if keepScanning {
		fmt.Println("160Error: Unexpected end of file")
		os.Exit(1)
	}
	fmt.Println("Parsed JSON:", data)
	os.Exit(0)
}
