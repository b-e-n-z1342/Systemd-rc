package main

import (
	"fmt"
	"os"
	"os/exec"
)
// runCommand — запускает команду и выводит stdout/stderr
func runCommand(name string, arg ...string) error {
	cmd := exec.Command(name, arg...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}


func main() {
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "Usage: systemctl <command> [service]")
		os.Exit(2)
	}
	command := os.Args[1]
	service := ""
	if len(os.Args) > 2 {
		service = os.Args[2]
	}
	switch command {
	// system
	case "poweroff":
		runCommand("poweroff")
	case "reboot":
		runCommand("reboot")
	case "suspend":
		runCommand("loginctl", "suspend")
	case "hibernate":
		runCommand("loginctl", "hibernate")
	default:
		fmt.Fprintf(os.Stderr, "Unknown command: %s\n", command)
		os.Exit(2)
	}
}

