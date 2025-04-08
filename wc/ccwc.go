package main
	
import (
    "fmt"
	"flag"
	"io"
    "os"
)

func countBytes(f *os.File) (int64) {
	offset, err := f.Seek(0, io.SeekEnd)
	if err != nil {
		panic(err)
	}
	return offset
}

func main() {
	cPtr := flag.Bool("c", false, "count number of bytes")
	flag.Parse()

	fileName := flag.Arg(0)

	f, err := os.Open(fileName)
	if err != nil {
		panic(err)
	}
	
	if *cPtr {
		fmt.Println(countBytes(f), fileName)
	}
	
	defer f.Close()
}
