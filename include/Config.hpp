#pragma once

#include <string>
#include "Browser.hpp"

class ConfigManager {
  public:
    static int init();
    static void save();
    static void set_pair(std::string key, std::string pair);
    static std::string get_value(std::string key);
    static void set_theme();
    static void set_mode();
    static void set_proxy();
    static void set_all();

    static void save_entry_info(Entry* entry);
    static void load_entry(Entry* entry);
};