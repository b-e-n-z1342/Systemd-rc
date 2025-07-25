# Systemd-rc


###### конвектор команд для Open-RC.
конвектор не эмулирует systemd, а только переводит команды для systemd в openrc 

## установка
```
git clone https://b-e-n-z1342/Systemd-rc.git
cd Systemd-rc
./install
```

## команды
``
systemctl {ebable/diseble/stop/start/status} <процесс>
``
## что под капотом

``
rc-update -- systemctl enable/disable --  default 
``
### _
``
rc-service -- systemctl start/status/stop --
``

## Где уже используется 

Этот скрипт уже используется в QuasarLinux 
https://b-e-n-z1342.github.io/
