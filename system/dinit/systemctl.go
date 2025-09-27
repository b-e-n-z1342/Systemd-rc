package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

// runCommand — запускает команду и выводит stdout/stderr
func runCommand(name string, arg ...string) error {
	cmd := exec.Command(name, arg...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}

// isServiceEnabled проверяет, включена ли служба в автозагрузку (runlevel 'default')
func isServiceEnabled(service string) bool {
	// Защита от пустого имени
	if service == "" {
		return false
	}

	// Имя службы не должно содержать слэшей или быть специальными именами
	if strings.ContainsAny(service, "/\\") || service == "." || service == ".." {
		return false
	}

	// Формируем путь безопасно
	path := filepath.Join("/etc/runlevels/default", service)

	// Проверяем существование как symlink (Lstat не следует по ссылке)
	info, err := os.Lstat(path)
	if err != nil {
		return false // файла нет → не включена
	}

	// В OpenRC службы — это символические ссылки
	return info.Mode()&os.ModeSymlink != 0
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
		// unit
		case "enable":
			if service == "" {
				fmt.Fprintln(os.Stderr, "error: service name required")
				os.Exit(2)
			}
			runCommand("dinitctl", "enable", service)

		case "disable":
			if service == "" {
				fmt.Fprintln(os.Stderr, "error: service name required")
				os.Exit(2)
			}
			runCommand("dinitctl", "disable", service)

		case "status":
			if service == "" {
				fmt.Fprintln(os.Stderr, "error: service name required")
				os.Exit(2)
			}
			runCommand("dinitctl", "status", service)

		case "start":
			if service == "" {
				fmt.Fprintln(os.Stderr, "error: service name required")
				os.Exit(2)
			}
			runCommand("dinitctl", "start", service)

		case "stop":
			if service == "" {
				fmt.Fprintln(os.Stderr, "error: service name required")
				os.Exit(2)
			}
			runCommand("dinitctl", "stop", service)

		case "reload":
			if service == "" {
				fmt.Fprintln(os.Stderr, "error: service name required")
				os.Exit(2)
			}
			runCommand("dinitctl", "reload", service)

		case "restart":
			if service == "" {
				fmt.Fprintln(os.Stderr, "error: service name required")
				os.Exit(2)
			}
			runCommand("dinitctl", "restart", service)

		case "list-units":
			runCommand("dinitctl", "list")

		// system 

		case "halt":
			runCommand("halt")

		case "poweroff":
			runCommand("poweroff")

		case "reboot":
			runCommand("reboot")

		case "is-enabled": // ← правильное имя команды!
			if service == "" {
				fmt.Fprintln(os.Stderr, "error: service name required")
				os.Exit(2)
			}
			if isServiceEnabled(service) {
				fmt.Println("enabled")
				os.Exit(0)
			} else {
				fmt.Println("disabled")
				os.Exit(1)
			}

		case "list-unit-files":
			runCommand("dinitctl", "show")

			// elogind section
		case "suspend":
			runCommand("loginctl", "suspend")

		case "hibernate":
			runCommand("loginctl", "hibernate")

		default:
			fmt.Fprintf(os.Stderr, "Unknown command: %s\n", command)
			os.Exit(2)
	}
}
