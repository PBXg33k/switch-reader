#pragma once

#include "Shared.hpp"

class Keyboard : public Handler {
  public:
    static void setup(HandlerEnum handle, std::string start_str = "");
    static void set_touch();
    static HandlerEnum on_event(int val);
    static void render();
    static std::string text;
  private:
    static int keyboard_x, keyboard_y;
    static int box_size;
    static int gap;
    static int caps_lock;

    static int active_elem;
    static int search_flags;
};