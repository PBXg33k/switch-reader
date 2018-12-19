#pragma once

#include <string>
#include <utility>
#include <vector>
#include <json-c/json.h>
#include "Browser.hpp"
#include "Domain.hpp"

namespace HSearch {
  static Domain default_domain;

  void expand_search(std::string completeURL, int page);
  void search_keywords(std::string keywords);
  
  void delete_domain(std::string name);
  void register_domain(std::string name, Domain* domain);
  Domain* current_domain();
  std::map<std::string, Domain*> get_domains();
}