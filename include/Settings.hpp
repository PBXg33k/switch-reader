#pragma once

#include "Shared.hpp"

class Settings : public Handler {
  public:
    static void set_touch();
    static HandlerEnum on_event(int val);
    static void render();
    static void finger_down(SDL_Event e);
};