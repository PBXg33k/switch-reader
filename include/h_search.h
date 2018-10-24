#ifndef H_SEARCH_H
#define H_SEARCH_H

#include "browser.h"
#include "api.h"
#include <vector>
#include <cstring>

class HSearch{
  public:
    static std::vector<Entry> search_keywords(char* keywords, int maxResults);

};

#endif
