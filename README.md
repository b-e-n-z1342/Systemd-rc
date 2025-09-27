# Systemd-rc


###### конвектор команд для Open-RC.
конвектор не эмулирует systemd, а только переводит команды для systemd в openrc 

## установка
```
git clone https://github.com/b-e-n-z1342/Systemd-rc.git
cd Systemd-rc
./install
```

## команды
``
systemctl {ebable/diseble/stop/start/status/restart/list} <процесс>
``
## что под капотом

команды:

-  rc-update -- systemctl enable/disable --  default 

-  rc-service -- systemctl start/status/stop -- 


## Где уже используется 

этот скрипт скоро будет использоваться в QuasarLinux 
  - https://b-e-n-z1342.github.io/QuasarLinux 
