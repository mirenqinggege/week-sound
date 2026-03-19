#ifndef WEEK_SOUND_CONFIG_H
#define WEEK_SOUND_CONFIG_H

#include <string>

struct ConfigData {
    int frequency = 20;
    double amplitude = 0.01;
    std::string log_file;
};

class Config {
public:
    bool load(const std::string& config_path);
    const ConfigData& data() const { return data_; }
    static std::string get_default_config_path();

private:
    std::string trim(const std::string& str);
    std::string get_default_log_path();

    ConfigData data_;
};

#endif // WEEK_SOUND_CONFIG_H
