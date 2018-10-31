#include <stdio.h>
#include <iostream>

#include "Ui.hpp"
#include "Shared.hpp"
#include "Api.hpp"
#include "Browser.hpp"
#include "Touch.hpp"
#include "Gallery.hpp"
#include "HSearch.hpp"
#include "Search.hpp"
#include "Config.hpp"

static SDL_Event* customEvent;
static int state;
const int joy_val[24] = {102, 101, 110, 111, 0, 0, 0, 0, 103, 104, 0, 0, 123, 120, 121, 122, 123, 120, 121, 122, 0, 0, 0, 0};

void Browser::quit_app(){
  state = 0;
}

int main(int argc, char **argv)
{
  Handler handler = Handler::Browser;

  Screen::init();
  ApiManager::init();

  nxlinkStdio();

  ConfigManager::init();
  ConfigManager::save();

  // Custom SDL event for timer
  Uint32 renderEvent = SDL_RegisterEvents(1);
  customEvent = new SDL_Event();
  SDL_memset(customEvent, 0, sizeof(SDL_Event));
  customEvent->type = renderEvent;
  customEvent->user.code = 0;
  customEvent->user.data1 = 0;
  customEvent->user.data2 = 0;

  // Set up joycons
  for (int i = 0; i < 2; i++) {
    if (SDL_JoystickOpen(i) == NULL) {
      SDL_Log("SDL_JoystickOpen: %s\n", SDL_GetError());
      SDL_Quit();
      return -1;
    }
  }

  Browser::set_touch();

  unsigned int last_time = 0, current_time;

  HSearch::search_keywords("", 25, (int)Category::NonH);

  state = 1;

  SDL_Event event;
  while(state){
    while(SDL_PollEvent(&event)){
      int val = -1;
      int x, y;
      switch (event.type) {
        case SDL_JOYBUTTONDOWN:
          val = joy_val[event.jbutton.button];
          break;
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
        // Browser scrolling
        case SDL_FINGERMOTION:
          // Don't move on press
          if(handler == Handler::Browser)
            Browser::scroll(event.tfinger.dx);
          break;
        case SDL_KEYDOWN:
          state = 0;
          break;
        default:
          break;
      }
      if(val > -1){
        std::cout << "Event: " << val << std::endl;
      }

      // Post Event to active handler
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

      // Render to active handler
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

      uint64_t memoryUsed = 0;
      svcGetInfo(&memoryUsed, 7, 0xffff8001, 0);
      Screen::draw_text_centered(std::to_string(memoryUsed), screen_width / 2, screen_height - 30, 400, 30, ThemeText, Screen::gallery_info);

      // Check for image thread updates
      ApiManager::update();

      Screen::render();
    }

    // Update Rate 20Hz - Keeps checks on image thread running
    current_time = SDL_GetTicks();
    if(current_time > last_time + 50){
      last_time = current_time;
      SDL_PushEvent(customEvent);
    }
  }

  //Browser::close();
  //GalleryBrowser::close();
  Screen::close();
  ApiManager::close();

  return 0;
}
