#pragma once

#include "Shared.hpp"
#include <string>

class SearchBrowser {
  public:
    static void set_touch();
    static Handler on_event(int val);
    static void render();
  private:
    static int keyboard_x, keyboard_y;
    static int box_size;
    static int gap;
    static int caps_lock;

    static int active_elem;
    static std::string search_str;
    static int search_flags;
};
