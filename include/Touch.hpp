#pragma once

#include "SDL2/SDL.h"
#include <vector>
#include <utility>

using namespace std;

class TouchManager {
  public:
    static void add_bounds(int x, int y, int w, int h, int val);
    static int get_value(int x, int y);
    static void clear();
  private:
    static vector<pair<SDL_Rect, int>> bounds;
};
