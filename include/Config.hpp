#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class ConfigManager {
  public:
    static int init();
    static void save();
    static void set_pair(std::string key, std::string pair);
    static std::string get_value(std::string key);
};

#endif
