// Systemd-rc
// FreeBSD version
// by QuasarFoks

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
using namespace std;

// Прототипы функций
void show_help();
void list_services();
bool service_exists(const string& service_name);
void enable_service(const string& service_name);
void disable_service(const string& service_name);
void check_service(const string& service_name);
void power_management(const string& command);
int safe_system(const vector<string>& args);

// Безопасное выполнение команды с аргументами
int safe_system(const vector<string>& args) {
    if (args.empty()) return -1;

    // Собираем команду с экранированием
    string safe_command;
    for (const auto& arg : args) {
        // Экранируем специальные символы shell
        string safe_arg = "'";
        for (char c : arg) {
            if (c == '\'') safe_arg += "'\\''";  // экранируем кавычки
            else safe_arg += c;
        }
        safe_arg += "'";

        if (!safe_command.empty()) safe_command += " ";
        safe_command += safe_arg;
    }

    return system(safe_command.c_str());
}

bool service_exists(const string& service_name) {
    string command = "service " + service_name + " status 2>/dev/null";
    int result = system(command.c_str());
    return (result != 32512);
}

// Команды питания с безопасным выполнением
void power_management(const string& command) {
    vector<string> cmd_args;

    if (command == "poweroff") {
        cmd_args = {"shutdown", "-p", "now"};
    }
    else if (command == "reboot") {
        cmd_args = {"shutdown", "-r", "now"};
    }
    else if (command == "halt") {
        cmd_args = {"shutdown", "-h", "now"};
    }
    else if (command == "suspend") {
        cmd_args = {"acpiconf", "-s", "3"};
    }
    else if (command == "hibernate") {
        cout << "[error]: Гибернация не поддерживается в этой версии" << endl;
        return;
    }
    else {
        cout << "[error]: Неизвестная команда питания: " << command << endl;
        return;
    }

    // Выполняем безопасно
    cout << "[Info]: Выполняется: " << command << endl;
    int result = safe_system(cmd_args);

    if (result != 0) {
        cout << "[error]: Ошибка выполнения команды" << endl;
    }
}

void enable_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[error]: Service is not found" << endl;
        return;
    }

    cout << "[OK]: Добавляем " << service_name << " в автозагрузку..." << endl;

    string check_cmd = "grep '^" + service_name + "_enable=\"YES\"' /etc/rc.conf";
    if (system(check_cmd.c_str()) == 0) {
        cout << "[Warring]: Сервис " << service_name << " уже в автозагрузке" << endl;
        return;
    }

    string command = "echo '" + service_name + "_enable=\"YES\"' | tee -a /etc/rc.conf 2>/dev/null";
    int result = system(command.c_str());

    if (result == 0) {
        cout << "[OK] " << service_name << "_enable=\"YES\" добавлено в /etc/rc.conf" << endl;
    } else {
        cout << "[error]: Ошибка записи в /etc/rc.conf" << endl;
    }
}



void disable_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[error]: Service is not found" << endl;
        return;
    }

    cout << "[OK]: Убераем " << service_name << " из автозагрузку..." << endl;

    string check_cmd = "grep '^" + service_name + "_enable=\"YES\"' /etc/rc.conf";
    if (system(check_cmd.c_str()) != 0) {
        cout << "[Warring]: Сервис " << service_name << " не в автозагрузке" << endl;
        return;
    }
    string command = "sed -i '' '/^" + service_name + "_enable=/d' /etc/rc.conf";
    int result = system(command.c_str());

    if (result == 0) {
        cout << "[OK] " << service_name << " убран из автозагрузки" << endl;
    } else {
        cout << "[error]: Ошибка удаления из /etc/rc.conf" << endl;
    }

}

void check_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[error]: Service " << service_name << " is not found" << endl;
        return;
    }

    // Заголовок как в systemctl
    cout << "[Info service] " << service_name << ".service - FreeBSD Service" << endl;

    // Проверяем автозагрузку
    string autostart_cmd = "grep '^" + service_name + "_enable=\"YES\"' /etc/rc.conf";
    int autostart_result = system(autostart_cmd.c_str());

    if (autostart_result == 0) {
        cout << "   Loaded: loaded (/etc/rc.conf; enabled)" << endl;
    } else {
        cout << "   Loaded: loaded (/etc/rc.conf; disabled)" << endl;
    }

    // Проверяем активен ли сервис
    string active_cmd = "service " + service_name + " status > /dev/null 2>&1";
    int active_result = system(active_cmd.c_str());

    if (active_result == 0) {
        cout << "   Active: active (running)" << endl;
    } else {
        cout << "   Active: inactive (dead)" << endl;
    }

    // Выводим полный статус
    cout << "   Status:" << endl;
    string status_cmd = "service " + service_name + " status";
    system(status_cmd.c_str());

    // Документация (как в systemctl)
    cout << "   Docs: man:" << service_name << "(8)" << endl;
}




void list_services() {
    cout << "UNIT                LOAD   ACTIVE   SUB     DESCRIPTION" << endl;
    cout << "---                 ---    ------   ---     -----------" << endl;

    // Команда для получения списка сервисов
    string command =
    "grep '_enable=\"YES\"' /etc/rc.conf | "
    "sed 's/_enable.*//' | "
    "while read service; do "
    "  if service \"$service\" status >/dev/null 2>&1; then "
    "    printf \"%-20s %-6s %-8s %-7s %s\\n\" \"$service.service\" \"loaded\" \"active\" \"running\" \"FreeBSD Service\"; "
    "  else "
    "    printf \"%-20s %-6s %-8s %-7s %s\\n\" \"$service.service\" \"loaded\" \"inactive\" \"dead\" \"FreeBSD Service\"; "
    "  fi; "
    "done";

    system(command.c_str());
}

void is_enabled_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[error]: Service " << service_name << " is not found" << endl;
        return;
    }

    // Проверяем автозагрузку
    string check_cmd = "grep '^" + service_name + "_enable=\"YES\"' /etc/rc.conf";
    int result = system(check_cmd.c_str());

    if (result == 0) {
        cout << "enabled" << endl;
    } else {
        cout << "disabled" << endl;
    }
}

void start_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[error]: Service " << service_name << " is not found" << endl;
        return;
    }

    cout << "[OK]: Starting " << service_name << "..." << endl;

    string command = "service " + service_name + " start";
    int result = system(command.c_str());

    if (result == 0) {
        cout << "[OK] " << service_name << " started successfully" << endl;
    } else {
        cout << "[error]: Failed to start " << service_name << endl;
    }
}

void stop_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[error]: Service " << service_name << " is not found" << endl;
        return;
    }

    cout << "[OK]: Stopping " << service_name << "..." << endl;

    string command = "service " + service_name + " stop";
    int result = system(command.c_str());

    if (result == 0) {
        cout << "[OK] " << service_name << " stopped successfully" << endl;
    } else {
        cout << "[error]: Failed to stop " << service_name << endl;
    }
}

void restart_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[error]: Service " << service_name << " is not found" << endl;
        return;
    }

    cout << "[OK]: Restarting " << service_name << "..." << endl;

    string command = "service " + service_name + " restart";
    int result = system(command.c_str());

    if (result == 0) {
        cout << "[OK] " << service_name << " restarted successfully" << endl;
    } else {
        cout << "[error]: Failed to restart " << service_name << endl;
    }
}

void reload_service(const string& service_name) {
    if (!service_exists(service_name)) {
        cout << "[error]: Service " << service_name << " is not found" << endl;
        return;
    }

    cout << "[OK]: Reloading " << service_name << "..." << endl;

    // Для некоторых сервисов может быть reload, для других - restart
    string command = "service " + service_name + " reload 2>/dev/null || service " + service_name + " restart";
    int result = system(command.c_str());

    if (result == 0) {
        cout << "[OK] " << service_name << " reloaded successfully" << endl;
    } else {
        cout << "[error]: Failed to reload " << service_name << endl;
    }
}

void show_help() {
    cout << "systemctl - FreeBSD Service Manager" << endl;
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
