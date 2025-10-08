package main

import (
	"bufio"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

const (
	runitLogBase = "/var/log"
)

// parseRunitTimestamp парсит время из строки вида "@1712345678 ..."
func parseRunitTimestamp(line string) time.Time {
	if !strings.HasPrefix(line, "@") {
		return time.Time{}
	}
	end := strings.IndexAny(line[1:], " \n")
	if end == -1 {
		end = len(line) - 1
	} else {
		end += 1
	}
	tsStr := line[1:end]
	unixTs, err := strconv.ParseInt(tsStr, 10, 64)
	if err != nil {
		return time.Time{}
	}
	return time.Unix(unixTs, 0)
}

// readRunitLogs читает логи runit-сервиса
func readRunitLogs(logFile string, follow bool, since time.Time) error {
	file, err := os.Open(logFile)
	if err != nil {
		return fmt.Errorf("не удалось открыть %s: %w", logFile, err)
	}
	defer file.Close()

	reader := bufio.NewReader(file)

	// Если не follow — читаем с начала
	if !follow {
		for {
			line, err := reader.ReadString('\n')
			if err == io.EOF {
				break
			}
			if err != nil {
				return err
			}

			// Фильтр по времени
			if !since.IsZero() {
				if ts := parseRunitTimestamp(line); !ts.IsZero() && ts.Before(since) {
					continue
				}
			}

			// Убираем @timestamp для читаемости (опционально)
			if strings.HasPrefix(line, "@") {
				if idx := strings.Index(line, " "); idx != -1 {
					line = line[idx+1:]
				}
			}
			fmt.Print(line)
		}
		return nil
	}

	// Режим follow: сначала вывести всё, потом следить
	for {
		line, err := reader.ReadString('\n')
		if err == io.EOF {
			break
		}
		if err != nil {
			return err
		}

		if !since.IsZero() {
			if ts := parseRunitTimestamp(line); !ts.IsZero() && ts.Before(since) {
				continue
			}
		}

		if strings.HasPrefix(line, "@") {
			if idx := strings.Index(line, " "); idx != -1 {
				line = line[idx+1:]
			}
		}
		fmt.Print(line)
	}

	// Следим за новыми записями
	for {
		line, err := reader.ReadString('\n')
		if err == io.EOF {
			time.Sleep(500 * time.Millisecond)
			continue
		}
		if err != nil {
			return err
		}

		if strings.HasPrefix(line, "@") {
			if idx := strings.Index(line, " "); idx != -1 {
				line = line[idx+1:]
			}
		}
		fmt.Print(line)
	}
}

func main() {
	var (
		unit   string
		follow bool
		kernel bool
		since  string
	)

	flag.StringVar(&unit, "u", "", "Служба (обязательна для runit)")
	flag.BoolVar(&follow, "f", false, "Следить за логом в реальном времени")
	flag.BoolVar(&kernel, "k", false, "Показать лог ядра (dmesg)")
	flag.StringVar(&since, "since", "", "С какого времени (например, '1h', '30m')")
	flag.Parse()

	if kernel {
		dmesg, _ := os.ReadFile("/var/log/dmesg")
		os.Stdout.Write(dmesg)
		return
	}

	if unit == "" {
		fmt.Fprintln(os.Stderr, "Ошибка: флаг -u <служба> обязателен")
		os.Exit(1)
	}

	logFile, err := getRunitLogPath(unit)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Ошибка: %v\n", err)
		os.Exit(1)
	}

	var sinceTime time.Time
	if since != "" {
		var duration time.Duration
		if strings.HasSuffix(since, "h") {
			if h, err := strconv.Atoi(strings.TrimSuffix(since, "h")); err == nil {
				duration = time.Duration(h) * time.Hour
			}
		} else if strings.HasSuffix(since, "m") {
			if m, err := strconv.Atoi(strings.TrimSuffix(since, "m")); err == nil {
				duration = time.Duration(m) * time.Minute
			}
		}
		if duration > 0 {
			sinceTime = time.Now().Add(-duration)
		}
	}

	if err := readRunitLogs(logFile, follow, sinceTime); err != nil {
		fmt.Fprintf(os.Stderr, "Ошибка: %v\n", err)
		os.Exit(1)
	}
}

// getRunitLogPath возвращает путь к current-логу службы
func getRunitLogPath(unit string) (string, error) {
	logPath := filepath.Join(runitLogBase, unit, "current")
	if _, err := os.Stat(logPath); os.IsNotExist(err) {
		return "", fmt.Errorf("лог для службы %q не найден (проверьте /var/log/%s/)", unit, unit)
	}
	return logPath, nil
}
