#pragma once

#include "Shared.hpp"
#include <string>

class SearchBrowser : public Handler {
  public:
    static void set_touch();
    static HandlerEnum on_event(int val);
    static void render();
    static std::string search_str;
  private:
    static int keyboard_x, keyboard_y;
    static int box_size;
    static int gap;
    static int caps_lock;

    static int active_elem;
};
