#pragma once

#include "SDL2/SDL.h"
#include <vector>
#include <utility>

class TouchManager {
  public:
    static TouchManager instance;

    void add_bounds(int x, int y, int w, int h, int val);
    int get_value(int x, int y);
    void clear();
  private:
    std::vector<std::pair<SDL_Rect, int>> bounds;
};