#pragma once

#include <string>
#include "Browser.hpp"

namespace ConfigManager {
  int init();
  void save();
  void set_pair(std::string key, std::string pair);
  std::string get_value(std::string key);
  void set_theme();
  void set_mode();
  void set_proxy();
  void set_all();

  void save_entry_info(Entry* entry);
  void load_entry(Entry* entry);

  static const std::string downloadsDir = "/switch/Reader/Downloads";
};