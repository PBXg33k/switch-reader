#include <stdio.h>

#include "ui.h"
#include "shared.h"
#include "api.h"
#include "Browser.h"
#include "TouchManager.h"
#include "Gallery.h"
#include "h_search.h"

int main(int argc, char **argv)
{
  Handler handler = Handler::Browser;

  ApiManager::init();

  //nxlinkStdio();

  Screen::init();
  Browser::set_touch();

  int state = 1;

  std::vector<Entry> result = HSearch::search_keywords("cat", 1);
  for(auto entry : result){
    printf("Returned %s\n", entry.url.c_str());
    Browser::add_entry(entry);
  }

  SDL_Event event;
  while(state){
    while(SDL_PollEvent(&event)){
      int val = -1;
      int x, y;
      switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
          SDL_GetMouseState(&x, &y);
          val = TouchManager::get_value(x, y);
          if(val == 100)
            state = 0;
          break;
        case SDL_FINGERDOWN:
          val = TouchManager::get_value(event.tfinger.x*1280, event.tfinger.y*720);
          if(val == 100)
            state = 0;
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
      if(val > -1){
        printf("Event : %d\n", val);
      }

      switch(handler){
        case Handler::Browser:
          handler = Browser::on_event(val);
          Browser::render();
          break;
        case Handler::Gallery:
          handler = GalleryBrowser::on_event(val);
          GalleryBrowser::render();
          break;
        default:
          break;
      }

      Screen::render();
    }
  }

  Screen::close();
  ApiManager::close();

  return 0;
}
