#ifndef TOUCHMANAGER_H
#define TOUCHMANAGER_H

#include "SDL2/SDL.h"
#include <vector>
#include <utility>

using namespace std;

class TouchManager {
  public:
    static void add_bounds(SDL_Rect, int val);
    static int get_value(int x, int y);
    static void clear();
  private:
    static vector<pair<SDL_Rect, int>> bounds;
};

#endif
