#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "Shared.hpp"

class SearchBrowser {
  public:
    static void set_touch();
    static Handler on_event(int val);
    static void render();
  private:
    static int active_elem;
};

#endif
