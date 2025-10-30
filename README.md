<!-- markdownlint-disable first-line-h1 -->
<!-- markdownlint-disable html -->
<!-- markdownlint-disable no-duplicate-header -->
<div align="center">

# Systemd-rc

![Static Badge](https://img.shields.io/badge/Статус-Активная_разработка-brightgreen?style=for-the-badge)

</div>
<div align="center">

[![Сайт](https://img.shields.io/badge/🌐-Официальный_сайт-2D2B55?style=for-the-badge&logo=google-chrome)](https://b-e-n-z1342.github.io/SystemdRC)
[![GitHub](https://img.shields.io/badge/💻-Исходный_код-FF6C37?style=for-the-badge&logo=gitlab)](https://github.com/b-e-n-z1342/Systemd-rc)

</div>

## Конвертер команд Systemd для других init-систем
Systemd-rc преобразует команды systemd в эквивалентные команды для других init-систем.

⚠️ **Важно**: утилита не эмулирует systemd, а только транслирует команды. На данный момент не поддерживается преобразование скриптов и конфигурационных файлов.
## установка
```
git clone https://github.com/b-e-n-z1342/Systemd-rc.git
cd Systemd-rc
./install
```

## команды
```
systemctl {enable|disable|stop|start|status|restart|list-unit|is-enabled} <служба>
## Управление питанием 
systemctl {poweroff|restart|halt|suspend|hibernate}
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
