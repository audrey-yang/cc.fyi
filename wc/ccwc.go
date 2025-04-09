package main
	
import (
    "fmt"
	"flag"
	"io"
    "os"
	"bufio"
)

func countBytes(f *os.File) (int64) {
	offset, err := f.Seek(0, io.SeekEnd)
	if err != nil {
		panic(err)
	}
	return offset
}

func countLines(f *os.File) (int64) {
	scanner := bufio.NewScanner(f)

	numLines := int64(0)
	for scanner.Scan() {
        numLines++
    }

    if err := scanner.Err(); err != nil {
        panic(err)
    }

	return numLines
}

func countWords(f *os.File) (int64) {
	scanner := bufio.NewScanner(f)
	scanner.Split(bufio.ScanWords)

	numWords := int64(0)
	for scanner.Scan() {
        numWords++
    }

    if err := scanner.Err(); err != nil {
        panic(err)
    }

	return numWords
}

func countChars(f *os.File) (int64) {
	scanner := bufio.NewScanner(f)
	scanner.Split(bufio.ScanRunes)

	numChars := int64(0)
	for scanner.Scan() {
        numChars++
    }

    if err := scanner.Err(); err != nil {
        panic(err)
    }

	return numChars
}

func main() {
	cPtr := flag.Bool("c", false, "count number of bytes")
	lPtr := flag.Bool("l", false, "count number of lines")
	wPtr := flag.Bool("w", false, "count number of words")
	mPtr := flag.Bool("m", false, "count number of characters")
	flag.Parse()

	fileName := flag.Arg(0)

	f, err := os.Open(fileName)
	if err != nil {
		panic(err)
	}
	
	if *cPtr {
		fmt.Println("  ", countBytes(f), fileName)
	} else if *lPtr {
		fmt.Println("  ", countLines(f), fileName)
	} else if *wPtr {
		fmt.Println("  ", countWords(f), fileName)
	} else if *mPtr {
		fmt.Println("  ", countChars(f), fileName)
	}
	
	defer f.Close()
}
