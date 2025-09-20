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
		runCommand("rc-update", "add", service, "default")
	case "disable":
		runCommand("rc-update", "del", service, "default")
	case "status":
		runCommand("rc-service", service, "status")
	case "start":
		runCommand("rc-service", service, "start")
	case "stop":
		runCommand("rc-service", service, "stop")
	case "reload":
		runCommand("rc-service", service, "reload")
	case "restart":
		runCommand("rc-service", service, "restart")
	case "list-units":
		runCommand("rc-service", "-l")
	case "halt":
		runCommand("halt")
	case "poweroff":
		runCommand("poweroff")
	case "reboot":
		runCommand("reboot")
	
	}
}
