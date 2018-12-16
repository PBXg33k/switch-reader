#pragma once

#include <string>
#include <map>
#include "Browser.hpp"

namespace ConfigManager {
  int init();
  void save();
  void set_pair(std::string key, std::string value);
  void add_pair(std::string key, std::string value);
  std::string get_value(std::string key);
  void remove_pair(std::string key, std::string value);

  void set_theme();
  void set_proxy();
  void set_all();
  std::multimap<std::string, std::string> get_all();
  void register_domains();

  void save_entry_info(Entry* entry);
  void load_entry(Entry* entry);

  static const std::string downloadsDir = "/switch/Reader/Downloads";
};