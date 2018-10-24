#include <stdio.h>

#include "ui.h"
#include "shared.h"
#include "api.h"
#include "Browser.h"
#include "TouchManager.h"
#include "h_search.h"

int main(int argc, char **argv)
{

  ApiManager::init();

  nxlinkStdio();

  Screen::init();
  Browser::set_touch();

  int state = 1;

  std::vector<Entry> result = HSearch::search_keywords("horse", 9);
  for(auto entry : result){
    Browser::add_entry(entry);
  }

  SDL_Event event;
  while(state){
    while(SDL_PollEvent(&event)){
      int val;
      switch (event.type) {
        case SDL_FINGERDOWN:
          val = TouchManager::get_value(event.tfinger.x*1280, event.tfinger.y*720);
          printf("%d - %d\n", event.tfinger.x*1280, event.tfinger.y*720);
          printf("%d\n",val);
          break;
        case SDL_KEYDOWN:
          state = 0;
          break;
        case SDL_JOYBUTTONDOWN:
          state = 0;
          break;
        default:
          break;
      }

      Browser::render();
      Screen::render();
    }
  }

  Screen::close();
  ApiManager::close();

  return 0;
}
