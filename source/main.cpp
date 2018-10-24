#include <stdio.h>

#include "ui.h"
#include "shared.h"
#include "api.h"
#include "browser.h"
#include "h_search.h"

int main(int argc, char **argv)
{

  ApiManager::init();
  nxlinkStdio();
  Screen::init();

  int state = 1;

  std::vector<Entry> result = HSearch::search_keywords("horse", 9);

  SDL_Event event;
  while(state){
    while(SDL_PollEvent(&event)){
      switch (event.type) {
        case SDL_KEYDOWN:
          state = 0;
          break;
        case SDL_JOYBUTTONDOWN:
          printf("Exiting");
          state = 0;
          break;
        default:
          break;
      }

      Browser::render_entry(&result[0], 30, 30);
      Browser::render_entry(&result[1], 30, 30 + (Browser::maxh) + (30));
      Browser::render_entry(&result[2], 30, 30 + (Browser::maxh * 2) + (30 * 2));
      Browser::render_entry(&result[3], 30 + (Browser::maxw2) + (30), 30);
      Browser::render_entry(&result[4], 30 + (Browser::maxw2) + (30), 30 + (Browser::maxh) + (30));
      Browser::render_entry(&result[5], 30 + (Browser::maxw2) + (30), 30 + (Browser::maxh * 2) + (30 * 2));
      Browser::render_entry(&result[6], 30 + (Browser::maxw2 * 2) + (30 * 2), 30);
      Browser::render_entry(&result[7], 30 + (Browser::maxw2 * 2) + (30 * 2), 30 + (Browser::maxh) + (30));
      Browser::render_entry(&result[8], 30 + (Browser::maxw2 * 2) + (30 * 2), 30 + (Browser::maxh * 2) + (30 * 2));
      Screen::render();
    }
  }

  Screen::close();
  ApiManager::close();

  return 0;
}
