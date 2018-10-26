#ifndef H_SEARCH_H
#define H_SEARCH_H

#include "Browser.h"
#include "api.h"
#include <vector>
#include <cstring>

class HSearch{
  public:
    static std::vector<Entry> search_keywords(std::string keywords, int maxResults);

};

#endif
