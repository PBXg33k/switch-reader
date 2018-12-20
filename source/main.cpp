#include <stdio.h>
#include <iostream>
#include <unistd.h>

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
#include "Dialog.hpp"

#define lookupURL "http://www.google.com"

static SDL_Event* customEvent;
static int state;
static HandlerEnum handler;
float motion_track;

void Browser::force_handler(HandlerEnum forced){
  handler = forced;
}

HandlerEnum Browser::get_handler(){
  return handler;
}

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
    default:
      break;
  }
}

int main(int argc, char **argv)
{
  handler = HandlerEnum::Browser;
  motion_track = 0;
  bool finger_was_down = false;

  // Domain setup
  Domain_EHentai* ex = new Domain_EHentai();
  ex->SearchURL = "https://exhentai.org/";
  ex->FavouritesURL = "https://exhentai.org/favorites.php";

  // Register domains
  HSearch::register_domain("E-Hentai", new Domain_EHentai());
  HSearch::register_domain("Exhentai", ex);
  HSearch::register_domain("NHentai", new Domain_NHentai());
  HSearch::register_domain("Local", new Domain_Local());

  // Initial setup
  ConfigManager::init();
  Screen::init();
  ApiManager::init();

  // Wait for internet connectivity
  int rc;
  CURL* curlLookup = curl_easy_init();
  curl_easy_setopt(curlLookup, CURLOPT_URL, lookupURL);
  curl_easy_setopt(curlLookup, CURLOPT_NOBODY, 1);

  while((rc = curl_easy_perform(curlLookup)) != CURLE_OK){
    Screen::clear(ThemeBG);
    Screen::draw_text_centered("Network still loading...", 0, 0, screen_width, screen_height, ThemeText, Screen::header);
    Screen::render();
    sleep(1);
  }

  curl_easy_cleanup(curlLookup);

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

  HSearch::search_keywords("");

  state = 1;

  SDL_Event event;
  while(state){
    while(SDL_PollEvent(&event)){
      int val = -1;
      int x, y;
      float moved;
      switch (event.type) {
        case SDL_JOYBUTTONDOWN:
          val = Shared::joy_val[event.jbutton.button];
          break;

        case SDL_MOUSEBUTTONDOWN:
          SDL_GetMouseState(&x, &y);
          val = TouchManager::instance.get_value(x, y);
          break;

        case SDL_FINGERDOWN:
          Handler::fingerTouches.insert(std::make_pair(event.tfinger.fingerId, FloatPoint {event.tfinger.x, event.tfinger.y}));
          finger_down(handler, event);
          finger_was_down = true;
          break;

        case SDL_FINGERUP:
          if(finger_was_down){
            if(motion_track < 0.01 + 0.09*(stod(ConfigManager::get_value("press_sens")) / 300)){
              printf("Motion Track %f\n", motion_track);
              val = TouchManager::instance.get_value(event.tfinger.x*screen_width, event.tfinger.y*screen_height);
            }
            // Remove finger from map
            if(Handler::fingerTouches.count(event.tfinger.fingerId))
              Handler::fingerTouches.erase(event.tfinger.fingerId);
            // Do event
            finger_up(handler, event);
            // If no fingers touching, reset motion track
            if(Handler::fingerTouches.empty())
              motion_track = 0;
            finger_was_down = false;
          }
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
      
      if(val == 100){
        if(Dialog::get_bool("Quit app?")){
          state = 0;
          break;
        }
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

  ConfigManager::save();
  ApiManager::close();
  Screen::close();

  return 0;
}