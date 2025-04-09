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

func main() {
	cPtr := flag.Bool("c", false, "count number of bytes")
	lPtr := flag.Bool("l", false, "count number of lines")
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
	}
	
	defer f.Close()
}
