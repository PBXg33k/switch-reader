#include "Touch.hpp"

TouchManager TouchManager::instance;

void TouchManager::add_bounds(int x, int y, int w, int h, int val){
  SDL_Rect rect{x,y,x+w,y+h};
  std::pair<SDL_Rect, int> pair(rect, val);
  bounds.push_back(pair);
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
  bounds.clear();
}
