package main

import (
	"fmt"
	"os"
	"os/exec"
)
func runCommand(name string, arg ...string) error {
	cmd := exec.Command(name, arg...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}
func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: systemd-rc <command> [service]")
		os.Exit(1)
	}

	command := os.Args[1]
	service := ""
	if len(os.Args) > 2 {
		service = os.Args[2]
	}
 
	switch command {
	case "enable":
		runCommand("sv", "enable", service)
	case "disable":
		runCommand("sv", "disable", service)
	case "status":
		runCommand("sv", "status", service)
	case "start":
		runCommand("sv", "up", service)
	case "stop":
		runCommand("sv", "down", service)
	case "reload":
		runCommand("sv", "hup", service)
	case "restart":
		runCommand("sv", "restart", service)
	case "list-units":
		runCommand("sv", "-l")
	case "halt":
		runCommand("halt")
	case "poweroff":
		runCommand("poweroff")
	case "reboot":
		runCommand("reboot")
	}
}
