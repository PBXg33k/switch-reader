#pragma once

#include <string>
#include <utility>
#include <vector>
#include <json-c/json.h>
#include "Browser.hpp"

class HSearch{
  public:
    static void expand_search(std::string completeURL, int page);
    static void search_keywords(std::string keywords, size_t maxResults, int categories);
    static void search_favourites(size_t maxResults);
    
  private:
    static std::vector<std::pair<std::string, std::string>> get_tags(json_object* json);
    static void fill_tags(Entry* entry, json_object* json);

};