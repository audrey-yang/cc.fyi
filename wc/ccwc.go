package main

import (
	"bufio"
	"bytes"
	"flag"
	"fmt"
	"io"
	"os"
)

func countAndPrint(r io.Reader, splits []bufio.SplitFunc) {
	for _, split := range splits {
		w := new(bytes.Buffer)
		tr := io.TeeReader(r, w)
		scanner := bufio.NewScanner(tr)

		scanner.Split(split)

		count := int64(0)
		for scanner.Scan() {
			count++
		}

		if err := scanner.Err(); err != nil {
			panic(err)
		}

		fmt.Printf("%8d", count)

		r = w
		w = new(bytes.Buffer)
	}
}

func main() {
	cPtr := flag.Bool("c", false, "count number of bytes")
	lPtr := flag.Bool("l", false, "count number of lines")
	wPtr := flag.Bool("w", false, "count number of words")
	mPtr := flag.Bool("m", false, "count number of characters")
	flag.Parse()

	var r io.ReadCloser
	var err error
	if flag.NArg() == 0 {
		r = os.Stdin
	} else if flag.NArg() == 1 {
		fileName := flag.Arg(0)
		r, err = os.Open(fileName)
		if err != nil {
			panic(err)
		}
		defer r.Close()
	} else {
		fmt.Println("Usage: ccwc [options] [file]")
		flag.PrintDefaults()
		os.Exit(1)
	}

	// handle no flags case (equivalent to -c -l -w)
	if !*cPtr && !*lPtr && !*wPtr && !*mPtr {
		*cPtr = true
		*lPtr = true
		*wPtr = true
	}

	splits := []bufio.SplitFunc{}

	if *lPtr {
		splits = append(splits, bufio.ScanLines)
	}

	if *wPtr {
		splits = append(splits, bufio.ScanWords)
	}

	if *cPtr {
		splits = append(splits, bufio.ScanBytes)
	}

	if *mPtr {
		splits = append(splits, bufio.ScanRunes)
	}

	countAndPrint(r, splits)
	if flag.NArg() == 1 {
		fmt.Printf(" %s\n", flag.Arg(0))
	}
}
