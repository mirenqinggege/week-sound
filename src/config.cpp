#include "config.h"
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

std::string Config::get_default_config_path() {
#ifdef _WIN32
    char path[MAX_PATH];
    if (GetEnvironmentVariableA("ProgramData", path, MAX_PATH) > 0) {
        return std::string(path) + "\\week-sound\\week-sound.conf";
    }
    return "C:\\ProgramData\\week-sound\\week-sound.conf";
#else
    return "/etc/week-sound/week-sound.conf";
#endif
}

bool Config::load(const std::string& config_path) {
    std::ifstream file(config_path);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    std::string current_section;

    while (std::getline(file, line)) {
        line = trim(line);

        // 跳过空行和注释
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        // 解析 section
        if (line[0] == '[' && line.back() == ']') {
            current_section = line.substr(1, line.size() - 2);
            continue;
        }

        // 解析键值对
        size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        if (current_section == "audio") {
            if (key == "frequency") {
                try {
                    data_.frequency = std::stoi(value);
                } catch (...) {}
            } else if (key == "amplitude") {
                try {
                    data_.amplitude = std::stod(value);
                } catch (...) {}
            }
        } else if (current_section == "general") {
            if (key == "log_file") {
                data_.log_file = value;
            }
        }
    }

    // 如果没有设置日志文件，使用默认路径
    if (data_.log_file.empty()) {
        data_.log_file = get_default_log_path();
    }

    return true;
}

std::string Config::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string Config::get_default_log_path() {
#ifdef _WIN32
    char path[MAX_PATH];
    if (GetEnvironmentVariableA("ProgramData", path, MAX_PATH) > 0) {
        return std::string(path) + "\\week-sound\\week-sound.log";
    }
    return "C:\\ProgramData\\week-sound\\week-sound.log";
#else
    return "/var/log/week-sound/week-sound.log";
#endif
}
