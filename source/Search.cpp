#include "Search.hpp"
#include "Keyboard.hpp"
#include "Ui.hpp"
#include "Touch.hpp"
#include "HSearch.hpp"
#include "Browser.hpp"
#include "Api.hpp"
#include "Config.hpp"
#include <tgmath.h>


int const SearchBrowser::gap = 20;
int const SearchBrowser::keyboard_x = 40;
int const SearchBrowser::keyboard_y = screen_height - 40;
int SearchBrowser::box_size = 75;
int SearchBrowser::active_elem = 0;
int SearchBrowser::caps_lock = 0;

static int field = 0;

std::string SearchBrowser::search_str;

void SearchBrowser::set_touch(){
  TouchManager::clear();

  // Return to Browser
  TouchManager::add_bounds(screen_width - 75, 0, 75, 75, 101);

  // Open Keywords Entry
  TouchManager::add_bounds(keyboard_x, 60, 680, 100, 102);

  // Open Language Entry
  TouchManager::add_bounds(keyboard_x, 260, 680, 100, 110);

  // Domain specific touch events
  Domain* domain = HSearch::current_domain();
  domain->search_touch();

}

HandlerEnum SearchBrowser::on_event(int val){
  if(val == 101){
    // Clear current search
    Browser::clear();
    
    HSearch::search_keywords(search_str.c_str());

    // Go back to Browser
    Browser::set_touch();
    return HandlerEnum::Browser;
  }

  // Open keyboard
  if(val == 102){
    field = 0;
    Keyboard::setup(HandlerEnum::Search, search_str);
    Keyboard::set_touch();
    return HandlerEnum::Keyboard;
  }
  
  // Open lang keyboard
  if(val == 110){
    field = 1;
    Keyboard::setup(HandlerEnum::Search, ConfigManager::get_value("lang"));
    Keyboard::set_touch();
    return HandlerEnum::Keyboard;
  }

  // Returned from Keyboard
  if(val == Shared::KeyboardReturn){
    switch(field){
      case 0:
        search_str = Keyboard::text;
        break;
      case 1:
        ConfigManager::set_pair("lang", Keyboard::text);
        break;
      default:
        break;
    }
    set_touch();
  }

  Domain* domain = HSearch::current_domain();
  return domain->search_event(val);
}

void SearchBrowser::render(){
  Screen::clear(ThemeBG);

  // Back button
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);

  // Render search string
  Screen::draw_text("Search", keyboard_x, 15, ThemeText, Screen::large);
  Screen::draw_button(keyboard_x, 60, 680, 100, ThemeButton, ThemeButtonBorder, 6);
  Screen::draw_text_centered(search_str, keyboard_x, 60, 680, 100, ThemeButtonText, Screen::large);

  // Render language selection
  Screen::draw_text("Language", keyboard_x, 215, ThemeText, Screen::large);
  Screen::draw_button(keyboard_x, 260, 680, 100, ThemeButton, ThemeButtonBorder, 6);
  Screen::draw_text_centered(ConfigManager::get_value("lang"), keyboard_x, 260, 680, 100, ThemeButtonText, Screen::large);

  Domain* domain = HSearch::current_domain();
  domain->search_render();

}
