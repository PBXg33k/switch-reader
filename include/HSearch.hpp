#pragma once

#include <string>

class HSearch{
  public:
    static void expand_search(std::string completeURL, int page);
    static void search_keywords(std::string keywords, size_t maxResults, int categories);

};