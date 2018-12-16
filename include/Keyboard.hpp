#pragma once

#include "Shared.hpp"
#include "Touch.hpp"

class Keyboard  {
  public:
    static std::string get_input(std::string msg, std::string start_str = "");
  private:
    static void set_touch(TouchManager* touch);
    static void on_event(int val);
    static void render();
    static std::string text;
    static std::string message;
    static int keyboard_x, keyboard_y;
    static int box_size;
    static int gap;
    static int caps_lock;

    static int active_elem;
    static int search_flags;
};