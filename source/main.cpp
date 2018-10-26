#include <stdio.h>

#include "Ui.hpp"
#include "Shared.hpp"
#include "Api.hpp"
#include "Browser.hpp"
#include "TouchManager.hpp"
#include "Gallery.hpp"
#include "HSearch.hpp"
#include "Search.hpp"

int main(int argc, char **argv)
{
  Handler handler = Handler::Browser;

  ApiManager::init();

  nxlinkStdio();

  Screen::init();

  Browser::set_touch();

  int state = 1;

  std::vector<Entry> result = HSearch::search_keywords("", 25, (int)Category::NonH);
  for(auto entry : result){
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

      // Post Event
      switch(handler){
        case Handler::Browser:
          handler = Browser::on_event(val);
          break;
        case Handler::Search:
          handler = SearchBrowser::on_event(val);
          break;
        case Handler::Gallery:
          handler = GalleryBrowser::on_event(val);
          break;
        default:
          break;
      }

      // render
      switch(handler){
        case Handler::Browser:
          Browser::render();
          break;
        case Handler::Search:
          SearchBrowser::render();
          break;
        case Handler::Gallery:
          GalleryBrowser::render();
          break;
        default:
          break;
      }


      Screen::render();
    }
  }

  //Browser::close();
  //GalleryBrowser::close();
  Screen::close();
  ApiManager::close();

  return 0;
}
