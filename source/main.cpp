#include <stdio.h>

#include "Ui.hpp"
#include "Shared.hpp"
#include "Api.hpp"
#include "Browser.hpp"
#include "Touch.hpp"
#include "Gallery.hpp"
#include "HSearch.hpp"
#include "Search.hpp"
#include "Config.hpp"

static SDL_Event* event;

int main(int argc, char **argv)
{
  Handler handler = Handler::Browser;

  Screen::init();

  ApiManager::init();

  nxlinkStdio();

  ConfigManager::init();

  Uint32 renderEvent = SDL_RegisterEvents(1);
  event = new SDL_Event();
  SDL_memset(event, 0, sizeof(SDL_Event));
  event->type = renderEvent;
  event->user.code = 0;
  event->user.data1 = 0;
  event->user.data2 = 0;

  Browser::set_touch();

  int state = 1;
  unsigned int last_time = 0, current_time;

  HSearch::search_keywords("", 25, (int)Category::NonH);

  SDL_Event polled;
  while(state){
    while(SDL_PollEvent(&polled)){
      int val = -1;
      int x, y;
      switch (polled.type) {
        case SDL_MOUSEBUTTONDOWN:
          SDL_GetMouseState(&x, &y);
          val = TouchManager::get_value(x, y);
          if(val == 100)
            state = 0;
          break;
        case SDL_FINGERDOWN:
          val = TouchManager::get_value(polled.tfinger.x*1280, polled.tfinger.y*720);
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
      ApiManager::update();
      Screen::render();
    }

    // Update 10Hz
    current_time = SDL_GetTicks();
    if(current_time > last_time + 100){
      last_time = current_time;
      SDL_PushEvent(event);
    }
  }

  //Browser::close();
  //GalleryBrowser::close();
  Screen::close();
  ApiManager::close();

  return 0;
}
