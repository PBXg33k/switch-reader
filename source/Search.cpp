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

std::string SearchBrowser::search_str;

void SearchBrowser::set_touch(){
  TouchManager::instance.clear();

  // Return to Browser
  TouchManager::instance.add_bounds(screen_width - 75, 0, 75, 75, 101);

  // Open Keywords Entry
  TouchManager::instance.add_bounds(keyboard_x, 60, 680, 100, 102);

  // Open Language Entry
  TouchManager::instance.add_bounds(keyboard_x, 260, 680, 100, 110);

  // Domain specific touch events
  Domain* domain = HSearch::current_domain();
  domain->search_touch();

}

HandlerEnum SearchBrowser::on_event(int val){
  if(val == 101){
    // Clear current search
    Browser::clear();
    
    HSearch::search_keywords(search_str.c_str());

    // Special check for midchange
    if(Browser::get_handler() != HandlerEnum::Search){
      return Browser::get_handler();
    }

    // Go back to Browser
    Browser::set_touch();
    return HandlerEnum::Browser;
  }

  // Get search words
  if(val == 102){
    search_str = Keyboard::get_input("Search", search_str);
  }
  
  // Edit language
  if(val == 110){
    ConfigManager::set_pair("lang", Keyboard::get_input("Language", ConfigManager::get_value("lang")));
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
  Screen::draw_text_centered(search_str, keyboard_x, 60, 680, 100, ThemeText, Screen::large);

  // Render language selection
  Screen::draw_text("Language", keyboard_x, 215, ThemeText, Screen::large);
  Screen::draw_button(keyboard_x, 260, 680, 100, ThemeButton, ThemeButtonBorder, 6);
  Screen::draw_text_centered(ConfigManager::get_value("lang"), keyboard_x, 260, 680, 100, ThemeText, Screen::large);

  Domain* domain = HSearch::current_domain();
  domain->search_render();

}
