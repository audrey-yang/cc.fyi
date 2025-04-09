package main
	
import (
    "fmt"
	"flag"
	"io"
    "os"
	"bufio"
)

func count(r io.Reader, split bufio.SplitFunc) (int64) {
	scanner := bufio.NewScanner(r)
	scanner.Split(split)

	count := int64(0)
	for scanner.Scan() {
        count++
    }

    if err := scanner.Err(); err != nil {
        panic(err)
    }

	return count
}

func countBytes(r io.Reader) (int64) {
	return count(r, bufio.ScanBytes)
}

func countLines(r io.Reader) (int64) {
	return count(r, bufio.ScanLines)
}

func countWords(r io.Reader) (int64) {
	return count(r, bufio.ScanWords)
}

func countChars(r io.Reader) (int64) {
	return count(r, bufio.ScanRunes)
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

	if (*lPtr) {
		fmt.Printf("%6d", countLines(r))
	}

	if (*wPtr) {
		fmt.Printf("%6d", countWords(r))
	}

	if (*cPtr) {
		fmt.Printf("%6d", countBytes(r))
	}

	if (*mPtr) {
		fmt.Printf("%6d", countChars(r))
	}

	if flag.NArg() == 1 {
		fmt.Printf("\t%s\n", flag.Arg(0))
	}
}
