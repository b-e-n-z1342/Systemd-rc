# Systemd-rc


### Конвектор команд Systemd в другие init.
Конвектор не эмулирует systemd, а только переводит команды для systemd в другие init. Он ещё не умеет переводить скрипты или файлы конфигурации. 

## установка
```
git clone https://github.com/b-e-n-z1342/Systemd-rc.git
cd Systemd-rc
./install
```

## команды
```
systemctl {ebable/ diseble / stop/ start / status / restart / list-unit / is-enable} <процесс>
## power
systemctl { poweroff / restart / hatl / suspend / hibernate}
```
journalctl:
```
-k                    --- ядро.
-u <сервис> -f        --- следит за логами.
--since 1h            --- Парсит 1 час 30m фильтрует по времени.   
```
### Зависимости

|runit|dinit|s6|openrc|
|-----|-----|----|----|
|go   |go   |go  |go  |
|elogind|elogind|elogind|elogind|

## Где уже используется 
[QuasarLinux](https://b-e-n-z1342.github.io/QuasarLinux)

[Wiki](https://github.com/b-e-n-z1342/Systemd-rc/wiki)

[Wiki по коду](https://github.com/b-e-n-z1342/Systemd-rc/wiki/devel)
