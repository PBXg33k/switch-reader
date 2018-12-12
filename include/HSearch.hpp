#pragma once

#include <string>
#include <utility>
#include <vector>
#include <json-c/json.h>
#include "Browser.hpp"
#include "Domain.hpp"

namespace HSearch {
  void expand_search(std::string completeURL, int page);
  void search_keywords(std::string keywords, int categories);

  void register_domain(std::string type, Domain* domain);
  Domain* current_domain();
};