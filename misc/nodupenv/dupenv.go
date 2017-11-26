// similar to env(1) but allows duplicate environment variables to be set
// this version differs from the C implementation in that Go itself will
// de-duplicate environment variables so the `dupenv a=x a=y dupenv` test
// will fail

package main

import (
	"flag"
	"fmt"
	"os"
	"os/exec"
	"strings"
	"syscall"
)

var h = flag.Bool("h", false, "print usage message")
var i = flag.Bool("i", false, "clear out existing environment")
var U = flag.Bool("U", false, "allow unsafe operations")

func main() {
	var execpos int = -1
	var newenv []string

	flag.Parse()

	if *h {
		fmt.Fprintf(os.Stderr, "Usage: dupenv [-i] [env=val ..] command [args ..]\n")
		os.Exit(64)
	}

	if !*i {
		newenv = os.Environ()
	}

	for i := 0; i < flag.NArg(); i++ {
		equalpos := strings.Index(flag.Arg(i), "=")
		if equalpos == -1 {
			execpos = i
			break
		}
		if equalpos == 0 && !*U {
			fmt.Fprintf(os.Stderr, "invalid environment variable '%s'\n", flag.Arg(i))
			os.Exit(1)
		}
		newenv = append(newenv, flag.Arg(i))
	}

	if execpos == -1 {
		for _, env := range newenv {
			fmt.Println(env)
		}
		os.Exit(0)
	}

	executable := flag.Arg(execpos)
	binary, err := exec.LookPath(executable)
	if err != nil {
		fmt.Fprintf(os.Stderr, "could not find '%s' in PATH", executable)
		os.Exit(1)
	}

	execErr := syscall.Exec(binary, flag.Args()[execpos:], newenv)
	if execErr != nil {
		fmt.Fprintf(os.Stderr, "exec failed: %v", execErr)
		os.Exit(1)
	}
}
