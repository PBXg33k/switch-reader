#ifndef HSEARCH_HPP
#define HSEARCH_HPP

#include "Browser.hpp"
#include "Api.hpp"
#include <vector>

class HSearch{
  public:
    static void search_keywords(std::string keywords, size_t maxResults, int categories);

};

#endif
