#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "Shared.hpp"
#include <string>

class SearchBrowser {
  public:
    static void set_touch();
    static Handler on_event(int val);
    static void render();
  private:
    static int box_size;
    static int gap;

    static int active_elem;
    static std::string search_str;
};

#endif
