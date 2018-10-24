#include "TouchManager.h"

vector<pair<SDL_Rect, int>> TouchManager::bounds = vector<pair<SDL_Rect, int>>();

void TouchManager::add_bounds(SDL_Rect rect, int val){
  pair<SDL_Rect, int> pair(rect, val);
  TouchManager::bounds.push_back(pair);
}

int TouchManager::get_value(int x, int y){
  for(auto pair : bounds) {
    SDL_Rect box = pair.first;
    // Check within bounds
    if(x > box.x)
      if(x < box.w)
        if(y > box.y)
          if(y < box.h)
            return pair.second;
  }

  return -1;
}

void TouchManager::clear(){
  TouchManager::bounds = vector<pair<SDL_Rect, int>>();
}
