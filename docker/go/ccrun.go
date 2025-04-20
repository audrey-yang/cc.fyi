package main

import (
	"fmt"
	"os"
	"os/exec"
)

func main() {
	if os.Args[1] == "run" {
		cmd := os.Args[2]
		args := os.Args[3:]

		call := exec.Command(cmd, args...)
		stdout, err := call.Output()
		returnCode := call.ProcessState.ExitCode()

		if err != nil {
			fmt.Println(err.Error())
		} else {
			fmt.Println(string(stdout))
		}

		os.Exit(returnCode)
	}

	os.Exit(1)
}
