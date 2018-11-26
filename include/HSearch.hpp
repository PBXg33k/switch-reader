#pragma once

#include <string>
#include <utility>
#include <vector>
#include <json-c/json.h>
#include "Browser.hpp"

namespace HSearch {
  void expand_search(std::string completeURL, int page);
  void search_keywords(std::string keywords, size_t maxResults, int categories);
  void search_favourites();
  
  std::vector<std::pair<std::string, std::string>> get_tags(json_object* json);
  int json_entries(std::vector<std::string> gids, std::vector<std::string> gtkns, std::vector<std::string> urls);
  void fill_tags(Entry* entry, json_object* json);
};