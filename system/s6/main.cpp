#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <cstdlib>

using namespace std;
namespace fs = filesystem;

// Вспомогательные функции
bool service_exists(const string& service_name) {
    return fs::exists("/etc/s6-overlay/s6-rc.d/" + service_name) ||
    fs::exists("/etc/services.d/" + service_name);
}

bool is_s6_rc_service(const string& service_name) {
    return fs::exists("/etc/s6-overlay/s6-rc.d/" + service_name);
}

bool is_legacy_service(const string& service_name) {
    return fs::exists("/etc/services.d/" + service_name);
}

// Проверка автозагрузки для s6-rc сервисов
bool is_s6_rc_enabled(const string& service_name) {
    return fs::exists("/etc/s6-overlay/s6-rc.d/user/contents.d/" + service_name);
}

// Проверка автозагрузки для legacy сервисов
bool is_legacy_enabled(const string& service_name) {
    return !fs::exists("/etc/services.d/" + service_name + "/down");
}

// Получить путь к live сервису
string get_service_live_path(const string& service_name) {
    if (fs::exists("/run/service/" + service_name)) {
        return "/run/service/" + service_name;
    }
    return "/var/run/s6/legacy-services/" + service_name;
}

// Основные функции управления
void enable_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[ERROR] Сервис '" << service_name << "' не найден" << endl;
        return;
    }

    cout << "[OK]: Добавляем " << service_name << " в автозагрузку..." << endl;

    if (is_s6_rc_service(service_name)) {
        // Метод s6-rc: добавляем в user bundle
        fs::create_directories("/etc/s6-overlay/s6-rc.d/user/contents.d");
        ofstream f("/etc/s6-overlay/s6-rc.d/user/contents.d/" + service_name);
        if (f.good()) {
            cout << "[OK] " << service_name << " добавлен в автозагрузку (s6-rc)" << endl;
        } else {
            cout << "[ERROR] Ошибка добавления в автозагрузку" << endl;
        }
    } else {
        // Legacy метод: удаляем файл down
        string down_path = "/etc/services.d/" + service_name + "/down";
        if (fs::exists(down_path)) {
            fs::remove(down_path);
        }
        cout << "[OK] " << service_name << " добавлен в автозагрузку (legacy)" << endl;
    }
}

void disable_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[ERROR]: Сервис '" << service_name << "' не найден" << endl;
        return;
    }

    cout << "[OK]: Убираем " << service_name << " из автозагрузки..." << endl;

    if (is_s6_rc_service(service_name)) {
        // Метод s6-rc: удаляем из user bundle
        string path = "/etc/s6-overlay/s6-rc.d/user/contents.d/" + service_name;
        if (fs::exists(path)) {
            fs::remove(path);
            cout << "[OK] " << service_name << " убран из автозагрузки (s6-rc)" << endl;
        } else {
            cout << "[WARNING]: Сервис " << service_name << " не был в автозагрузке" << endl;
        }
    } else {
        // Legacy метод: создаем файл down
        string down_path = "/etc/services.d/" + service_name + "/down";
        ofstream f(down_path);
        cout << "[OK] " << service_name << " убран из автозагрузки (legacy)" << endl;
    }
}

void check_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[ERROR]: Сервис '" << service_name << "' не найден" << endl;
        return;
    }

    // Заголовок
    cout << "● " << service_name << ".service - S6 Service" << endl;

    // Проверяем автозагрузку
    bool enabled = is_s6_rc_service(service_name) ? is_s6_rc_enabled(service_name)
    : is_legacy_enabled(service_name);
    if (enabled) {
        cout << "   Loaded: loaded (enabled; s6-rc)" << endl;
    } else {
        cout << "   Loaded: loaded (disabled; s6-rc)" << endl;
    }

    // Проверяем активность
    string live_path = get_service_live_path(service_name);
    string cmd = "s6-svstat " + live_path + " 2>/dev/null";
    int active_result = system(cmd.c_str());

    if (active_result == 0) {
        // Получаем детальный статус
        string status_cmd = "s6-svstat -o 'up (pid {pid}, {time}ms)' " + live_path;
        cout << "   Active: ";
        system(status_cmd.c_str());
        cout << endl;
    } else {
        cout << "   Active: inactive (dead)" << endl;
    }

    // Документация
    cout << "   Docs: /etc/s6-overlay/s6-rc.d/" << service_name << endl;
}

void list_services() {
    cout << "UNIT                LOAD   ACTIVE   SUB     DESCRIPTION" << endl;
    cout << "------------------- ------ -------- ------- --------------------" << endl;

    // Получаем список через s6-rc
    string cmd =
    "if command -v s6-rc-db >/dev/null 2>&1; then "
    "  s6-rc-db list all 2>/dev/null | while read service; do "
    "    state=$(s6-rc-db -c 'state $service' 2>/dev/null || echo 'down'); "
    "    if [ \"$state\" = 'up' ]; then "
    "      printf \"%-20s %-6s %-8s %-7s %s\\n\" \"$service\" \"loaded\" \"active\" \"running\" \"S6 Service\"; "
    "    else "
    "      printf \"%-20s %-6s %-8s %-7s %s\\n\" \"$service\" \"loaded\" \"inactive\" \"dead\" \"S6 Service\"; "
    "    fi; "
    "  done; "
    "fi";

    // Добавляем legacy сервисы
    string legacy_cmd =
    "if [ -d /run/service ]; then "
    "  ls -1 /run/service/ | while read service; do "
    "    if s6-svstat /run/service/$service >/dev/null 2>&1; then "
    "      up=$(s6-svstat -u /run/service/$service 2>/dev/null && echo 'up' || echo 'down'); "
    "      if [ \"$up\" = 'up' ]; then "
    "        printf \"%-20s %-6s %-8s %-7s %s\\n\" \"$service\" \"loaded\" \"active\" \"running\" \"S6 Legacy\"; "
    "      else "
    "        printf \"%-20s %-6s %-8s %-7s %s\\n\" \"$service\" \"loaded\" \"inactive\" \"dead\" \"S6 Legacy\"; "
    "      fi; "
    "    fi; "
    "  done; "
    "fi";

    system(cmd.c_str());
    system(legacy_cmd.c_str());
}

void is_enabled_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[ERROR]: Сервис '" << service_name << "' не найден" << endl;
        return;
    }

    bool enabled = is_s6_rc_service(service_name) ? is_s6_rc_enabled(service_name)
    : is_legacy_enabled(service_name);
    cout << (enabled ? "enabled" : "disabled") << endl;
}

void start_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[ERROR]: Сервис '" << service_name << "' не найден" << endl;
        return;
    }

    cout << "[OK]: Запускаем " << service_name << "..." << endl;

    string cmd;
    if (is_s6_rc_service(service_name)) {
        cmd = "s6-rc -u change " + service_name;
    } else {
        cmd = "s6-svc -u " + get_service_live_path(service_name);
    }

    int result = system(cmd.c_str());
    if (result == 0) {
        cout << "[OK] " << service_name << " успешно запущен" << endl;
    } else {
        cout << "[ERROR]: Ошибка запуска " << service_name << endl;
    }
}

void stop_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[ERROR]: Сервис '" << service_name << "' не найден" << endl;
        return;
    }

    cout << "[OK]: Останавливаем " << service_name << "..." << endl;

    string cmd;
    if (is_s6_rc_service(service_name)) {
        cmd = "s6-rc -d change " + service_name;
    } else {
        cmd = "s6-svc -d " + get_service_live_path(service_name);
    }

    int result = system(cmd.c_str());
    if (result == 0) {
        cout << "[OK] " << service_name << " успешно остановлен" << endl;
    } else {
        cout << "[ERROR]: Ошибка остановки " << service_name << endl;
    }
}

void restart_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[ERROR]: Сервис '" << service_name << "' не найден" << endl;
        return;
    }

    cout << "[OK]: Перезапускаем " << service_name << "..." << endl;

    string cmd = "s6-svc -r " + get_service_live_path(service_name);
    int result = system(cmd.c_str());

    if (result == 0) {
        cout << "[OK] " << service_name << " успешно перезапущен" << endl;
    } else {
        cout << "[ERROR]: Ошибка перезапуска " << service_name << endl;
    }
}

void reload_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[ERROR]: Сервис '" << service_name << "' не найден" << endl;
        return;
    }

    cout << "[OK]: Перезагружаем конфигурацию " << service_name << "..." << endl;

    // Отправляем SIGHUP для перезагрузки конфигурации
    string cmd = "s6-svc -h " + get_service_live_path(service_name) + " 2>/dev/null";
    int result = system(cmd.c_str());

    if (result == 0) {
        cout << "[OK] " << service_name << " конфигурация перезагружена" << endl;
    } else {
        // Если не поддерживает reload, пробуем перезапустить
        cout << "[WARNING]: Перезагрузка не поддерживается, перезапускаем..." << endl;
        restart_service(service_name);
    }
}

// Функция для управления питанием (reboot, shutdown и т.д.)
void power_management(const string& action) {
    cout << "[OK]: Выполняется " << action << "..." << endl;

    string cmd;
    if (action == "reboot") {
        cmd = "s6-svscanctl -r /";
    } else if (action == "poweroff" || action == "shutdown") {
        cmd = "s6-svscanctl -p /";
    } else if (action == "halt") {
        cmd = "s6-svscanctl -h /";
    } else {
        cout << "[ERROR]: Неизвестное действие: " << action << endl;
        return;
    }

    int result = system(cmd.c_str());
    if (result == 0) {
        cout << "[OK] Команда " << action << " отправлена" << endl;
    } else {
        cout << "[ERROR]: Ошибка выполнения " << action << endl;
    }
}

// Функция для показа логов
void logs_service(const string& service_name, bool follow = false) {
    if (!service_exists(service_name)) {
        cout << "[ERROR]: Сервис '" << service_name << "' не найден" << endl;
        return;
    }

    string log_path = "/var/log/s6-logs/" + service_name + "/current";
    if (!fs::exists(log_path)) {
        log_path = "/var/log/s6-logs/" + service_name + "/main/current";
    }

    if (!fs::exists(log_path)) {
        cout << "[ERROR]: Логи для сервиса не найдены" << endl;
        return;
    }

    string cmd = follow ? "tail -f " + log_path : "cat " + log_path;
    system(cmd.c_str());
}



void show_help() {
    cout << "systemctl - S6 Service Manager" << endl;
    cout << "Usage: systemctl [COMMAND] [SERVICE]" << endl;
    cout << endl;
    cout << "Service Commands:" << endl;
    cout << "  start SERVICE      Start service" << endl;
    cout << "  stop SERVICE       Stop service" << endl;
    cout << "  restart SERVICE    Restart service" << endl;
    cout << "  reload SERVICE     Reload service configuration" << endl;
    cout << "  enable SERVICE     Enable service autostart" << endl;
    cout << "  disable SERVICE    Disable service autostart" << endl;
    cout << "  status SERVICE     Show service status" << endl;
    cout << "  is-enabled SERVICE Check if service is enabled" << endl;
    cout << "  list-units         List all services" << endl;
    cout << endl;
    cout << "Power Management:" << endl;
    cout << "  poweroff           Power off the system" << endl;
    cout << "  reboot             Reboot the system" << endl;
    cout << "  halt               Halt the system" << endl;
    cout << "  suspend            Suspend the system" << endl;
    cout << endl;
    cout << "Other:" << endl;
    cout << "  help               Show this help" << endl;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        show_help();
        return 0;
    }

    string action = argv[1];

    // Команды питания (не требуют сервиса)
    if (action == "poweroff" || action == "reboot" ||
        action == "halt" || action == "suspend" || action == "hibernate") {
        power_management(action);
    return 0;
        }

        if (action == "list-units") {
            list_services();
            return 0;
        }

        if (action == "help") {
            show_help();
            return 0;
        }

        if (argc < 3) {
            cout << "[error]: для действия '" << action << "' укажите имя сервиса" << endl;
            show_help();
            return 1;
        }

        string service = argv[2];

        if (action == "enable") {
            enable_service(service);
        }
        else if (action == "disable") {
            disable_service(service);
        }
        else if (action == "status") {
            check_service(service);
        }
        else if (action == "is-enabled") {
            is_enabled_service(service);
        }
        else if (action == "start") {
            start_service(service);
        }
        else if (action == "stop") {
            stop_service(service);
        }
        else if (action == "restart") {
            restart_service(service);
        }
        else if (action == "reload") {
            reload_service(service);
        }
        else {
            cout << "[error]: Неизвестное действие: " << action << endl;
            show_help();
            return 1;
        }

        return 0;
}
