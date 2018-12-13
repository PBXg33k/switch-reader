#pragma once

#include "Shared.hpp"
#include <string>

class SearchBrowser : public Handler {
  public:
    static void set_touch();
    static HandlerEnum on_event(int val);
    static void render();
    static std::string search_str;
    static const int keyboard_x, keyboard_y;
    static const int gap;

  private:
    static int box_size;
    static int caps_lock;

    static int active_elem;
};
