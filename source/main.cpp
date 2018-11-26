#include <stdio.h>
#include <iostream>

#include "Ui.hpp"
#include "Shared.hpp"
#include "Api.hpp"
#include "Browser.hpp"
#include "Touch.hpp"
#include "Gallery.hpp"
#include "Preview.hpp"
#include "HSearch.hpp"
#include "Search.hpp"
#include "Config.hpp"
#include "Settings.hpp"
#include "Keyboard.hpp"

static SDL_Event* customEvent;
static int state;
const int joy_val[24] = {102, 101, 110, 111, 0, 0, 0, 0, 103, 104, 0, 0, 123, 120, 121, 122, 123, 120, 121, 122, 0, 0, 0, 0};
float motion_track;

void Browser::quit_app(){
  state = 0;
}

HandlerEnum Shared::do_event(HandlerEnum handler, int val){
  switch(handler){
    case HandlerEnum::Browser:
      handler = Browser::on_event(val);
      break;
    case HandlerEnum::Search:
      handler = SearchBrowser::on_event(val);
      break;
    case HandlerEnum::Gallery:
      handler = GalleryBrowser::on_event(val);
      break;
    case HandlerEnum::Settings:
      handler = Settings::on_event(val);
      break;
    case HandlerEnum::Preview:
      handler = GalleryPreview::on_event(val);
      break;
    case HandlerEnum::Keyboard:
      handler = Keyboard::on_event(val);
      break;
    default:
      break;
  }

  return handler;
}

void finger_down(HandlerEnum handler, SDL_Event e){

  switch(handler){
    case HandlerEnum::Browser:
      Browser::finger_down(e);
      break;
    case HandlerEnum::Search:
      SearchBrowser::finger_down(e);
      break;
    case HandlerEnum::Gallery:
      GalleryBrowser::finger_down(e);
      break;
    case HandlerEnum::Settings:
      Settings::finger_down(e);
      break;
    case HandlerEnum::Preview:
      GalleryPreview::finger_down(e);
      break;
    case HandlerEnum::Keyboard:
      Keyboard::finger_down(e);
      break;
    default:
      break;
  }
}

void finger_up(HandlerEnum handler, SDL_Event e){

  switch(handler){
    case HandlerEnum::Browser:
      Browser::finger_up(e);
      break;
    case HandlerEnum::Search:
      SearchBrowser::finger_up(e);
      break;
    case HandlerEnum::Gallery:
      GalleryBrowser::finger_up(e);
      break;
    case HandlerEnum::Settings:
      Settings::finger_up(e);
      break;
    case HandlerEnum::Preview:
      GalleryPreview::finger_up(e);
      break;
    case HandlerEnum::Keyboard:
      Keyboard::finger_up(e);
      break;
    default:
      break;
  }
}

void do_gesture(HandlerEnum handler, SDL_Event e){
  // Pinch
  if(fabs(e.mgesture.dDist) > 0.002){

    switch(handler){
    case HandlerEnum::Browser:
      Browser::gesture(e);
      break;
    case HandlerEnum::Search:
      SearchBrowser::gesture(e);
      break;
    case HandlerEnum::Gallery:
      GalleryBrowser::gesture(e);
      break;
    case HandlerEnum::Settings:
      Settings::gesture(e);
      break;
    case HandlerEnum::Preview:
      GalleryPreview::gesture(e);
      break;
    case HandlerEnum::Keyboard:
      Keyboard::gesture(e);
      break;
    default:
      break;
  }
  }


}

void do_scroll(HandlerEnum handler, SDL_Event e){

  switch(handler){
    case HandlerEnum::Browser:
      Browser::scroll(e.tfinger.dx, e.tfinger.dy);
      break;
    case HandlerEnum::Search:
      SearchBrowser::scroll(e.tfinger.dx, e.tfinger.dy);
      break;
    case HandlerEnum::Gallery:
      GalleryBrowser::scroll(e.tfinger.dx, e.tfinger.dy);
      break;
    case HandlerEnum::Settings:
      Settings::scroll(e.tfinger.dx, e.tfinger.dy);
      break;
    case HandlerEnum::Preview:
      GalleryPreview::scroll(e.tfinger.dx, e.tfinger.dy);
      break;
    case HandlerEnum::Keyboard:
      Keyboard::scroll(e.tfinger.dx, e.tfinger.dy);
      break;
    default:
      break;
  }
}

int main(int argc, char **argv)
{
  HandlerEnum handler = HandlerEnum::Browser;
  motion_track = 0;

  ConfigManager::init();
  Screen::init();
  ApiManager::init();
  Browser::load_username();

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

  HSearch::search_keywords("", 25, stoi(ConfigManager::get_value("categories")));

  state = 1;

  SDL_Event event;
  while(state){
    while(SDL_PollEvent(&event)){
      int val = -1;
      int x, y;
      float moved;
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
          Handler::fingerTouches.insert(std::make_pair(event.tfinger.fingerId, FloatPoint {event.tfinger.x, event.tfinger.y}));
          finger_down(handler, event);
          break;

        case SDL_FINGERUP:
          if(motion_track < 0.017){
            printf("Motion Track %f\n", motion_track);
            val = TouchManager::get_value(event.tfinger.x*1280, event.tfinger.y*720);
            if(val == 100)
              state = 0;
          }
          // Remove finger from map
          if(Handler::fingerTouches.count(event.tfinger.fingerId))
            Handler::fingerTouches.erase(event.tfinger.fingerId);
          // Do event
          finger_up(handler, event);
          // If no fingers touching, reset motion track
          if(Handler::fingerTouches.empty())
            motion_track = 0;
          break;

        // Browser scrolling
        case SDL_FINGERMOTION:
          moved = abs(event.tfinger.dx) + abs(event.tfinger.dy);
          if(moved < 0.1 && moved > 0.0011){
            motion_track += moved;
            do_scroll(handler, event);
          }
          break;

        case SDL_MULTIGESTURE:
          if(event.mgesture.dDist < 0.1 && event.mgesture.numFingers == 2){
            motion_track += abs(event.mgesture.dDist);
            do_gesture(handler, event);
          }
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

      handler = Shared::do_event(handler, val);

      // Update event at 30Hz
      if(event.type == customEvent->type){
        // Render to active handler
        switch(handler){
          case HandlerEnum::Browser:
            Browser::render();
            break;
          case HandlerEnum::Search:
            SearchBrowser::render();
            break;
          case HandlerEnum::Gallery:
            GalleryBrowser::render();
            break;
          case HandlerEnum::Settings:
            Settings::render();
            break;
          case HandlerEnum::Preview:
            GalleryPreview::render();
            break;
          case HandlerEnum::Keyboard:
            Keyboard::render();
            break;
          default:
            break;
        }

        ApiManager::update();

        Screen::render();
      }
    }

    // Update Rate 30Hz - Keeps checks on image thread running
    current_time = SDL_GetTicks();
    if(current_time > last_time + 33){
      last_time = current_time;
      SDL_PushEvent(customEvent);
    }
  }

  ApiManager::close();
  Screen::close();
  ConfigManager::save();

  return 0;
}