package main

import (
	"flag"
	"fmt"
	"os"
	"os/exec"
)

func main() {
	var (
		unit   string
		follow bool
		kernel bool
	)

	flag.StringVar(&unit, "u", "", "Show logs for the specified service")
	flag.BoolVar(&follow, "f", false, "Follow the log output")
	flag.BoolVar(&kernel, "k", false, "Show kernel messages")
	flag.Parse()

	if kernel {
		// journalctl -k → dmesg
		cmd := exec.Command("dmesg")
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		cmd.Run()
		return
	}

	// Определяем, откуда брать логи
	logFile := "/var/log/messages"
	if _, err := os.Stat("/var/log/rc.log"); err == nil {
		// Если есть rc.log — он содержит логи запуска служб
		logFile = "/var/log/rc.log"
	}

	if unit != "" {
		if follow {
			// journalctl -u nginx -f
			tailCmd := exec.Command("sh", "-c", fmt.Sprintf("tail -F %s | grep --line-buffered -i '%s'", logFile, unit))
			tailCmd.Stdout = os.Stdout
			tailCmd.Stderr = os.Stderr
			tailCmd.Run()
		} else {
			// journalctl -u nginx
			grepCmd := exec.Command("grep", "-i", unit, logFile)
			grepCmd.Stdout = os.Stdout
			grepCmd.Stderr = os.Stderr
			grepCmd.Run()
		}
		return
	}

	// journalctl (без аргументов)
	if follow {
		exec.Command("tail", "-F", logFile).Run()
	} else {
		exec.Command("cat", logFile).Run()
	}
}
