#ifndef H_SEARCH_H
#define H_SEARCH_H

#include "browser.h"
#include "api.h"

typedef struct HResult{
  vector<Entry> results;
  size_t len;
}HResult;

class HSearch{
  public:
    static HResult search_keywords(char* keywords);
};

#endif
