#ifndef H_SEARCH_H
#define H_SEARCH_H

#include "Browser.h"
#include "api.h"
#include <vector>
#include <cstring>

class HSearch{
  public:
    static void search_keywords(std::vector<Entry*>* result, char* keywords, int maxResults);

};

#endif
