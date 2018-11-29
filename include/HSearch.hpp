#pragma once

#include <string>
#include <utility>
#include <vector>
#include <json-c/json.h>
#include "Browser.hpp"

namespace HSearch {
  std::string build_nh_image(Entry* e, int page, std::string type, bool thumbnail = false);
  json_object* fetch_nh_gallery(Entry* e);
  json_object* get_json_obj(json_object* json, std::string key);
  void expand_search(std::string completeURL, int page);
  void search_keywords(std::string keywords, int categories);
  void search_eh_keywords(std::string keywords, int categories);
  void search_nh_keywords(std::string keywords);
  void search_eh_favourites();
  
  std::vector<std::pair<std::string, std::string>> get_tags(json_object* json);
  int json_entries(std::vector<std::string> gids, std::vector<std::string> gtkns, std::vector<std::string> urls);
  void fill_tags(Entry* entry, json_object* json);
};