#include "Search.hpp"
#include "Keyboard.hpp"
#include "Ui.hpp"
#include "Touch.hpp"
#include "HSearch.hpp"
#include "Browser.hpp"
#include "Api.hpp"
#include <tgmath.h>

int SearchBrowser::box_size = 75;
int SearchBrowser::gap = 20;
int SearchBrowser::keyboard_x = 40;
int SearchBrowser::keyboard_y = screen_height - 40;
int SearchBrowser::active_elem = 0;
int SearchBrowser::caps_lock = 0;

std::string SearchBrowser::search_str;
int SearchBrowser::search_flags = (int)Category::NonH;

void SearchBrowser::set_touch(){
  TouchManager::clear();

  // Return to Browser
  TouchManager::add_bounds(screen_width - 75, 0, 75, 75, 101);

  // Open Text Entry
  TouchManager::add_bounds(keyboard_x, 60, 680, 100, 102);

  // All 10 Categorys
  TouchManager::add_bounds(keyboard_x + 680 + 30, 10, 200, 50, 50);

  TouchManager::add_bounds(keyboard_x + 680 + 30, 10 + 50 + gap-12, 200, 50, 51);

  TouchManager::add_bounds(keyboard_x + 680 + 30, 10 + 2*(50 + gap-12), 200, 50, 52);

  TouchManager::add_bounds(keyboard_x + 680 + 30, 10 + 3*(50 + gap-12), 200, 50, 53);

  TouchManager::add_bounds(keyboard_x + 680 + 30, 10 + 4*(50 + gap-12), 200, 50, 54);

  TouchManager::add_bounds(keyboard_x + 680 + 250, 10, 200, 50, 55);

  TouchManager::add_bounds(keyboard_x + 680 + 250, 10 + 50 + gap-12, 200, 50, 56);

  TouchManager::add_bounds(keyboard_x + 680 + 250, 10 + 2*(50 + gap-12), 200, 50, 57);

  TouchManager::add_bounds(keyboard_x + 680 + 250, 10 + 3*(50 + gap-12), 200, 50, 58);

  TouchManager::add_bounds(keyboard_x + 680 + 250, 10 + 4*(50 + gap-12), 200, 50, 59);

}

Handler SearchBrowser::on_event(int val){

  // > 50 leaves 40-49 for special characters, if desired later
  if(val >= 50 && val < 60){
    int flag = (int)(pow(2, val%50));
    if(search_flags & flag)
      search_flags = search_flags & ~flag;
    else
      search_flags = search_flags | flag;
  }

  if(val == 101){
    // Clear current search
    Browser::clear();

    // Do search
    HSearch::search_keywords(search_str.c_str(), 25, (int)search_flags);

    // Go back to Browser
    Browser::set_touch();
    return Handler::Browser;
  }

  // Open keyboard
  if(val == 102){
    Keyboard::setup(Handler::Search, search_str);
    Keyboard::set_touch();
    return Handler::Keyboard;
  }

  // Returned from Keyboard
  if(val == Shared::KeyboardReturn){
    set_touch();
    search_str = Keyboard::text;
  }

  return Handler::Search;
}

void SearchBrowser::render(){
  Screen::clear(ThemeBG);

  // Render search string
  Screen::draw_button(keyboard_x, 60, 680, 100, ThemeButton, ThemeButtonBorder, 6);
  if(!search_str.empty())
    Screen::draw_text_centered(search_str, keyboard_x, 60, 680, 100, ThemeButtonText, Screen::large);

  // Back button
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);

  // TODO Render buttons
  SDL_Color state;

  if(search_flags & (int)Category::Doujinshi) state = ThemeOptionSelected; else state = ThemeOptionUnselected;
  Screen::draw_button(keyboard_x + 680 + 30, 10, 200, 50, state, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Doujinshi", keyboard_x + 680 + 30, 10, 200, 50, ThemeButtonText, Screen::normal);

  if(search_flags & (int)Category::Manga) state = ThemeOptionSelected; else state = ThemeOptionUnselected;
  Screen::draw_button(keyboard_x + 680 + 30, 10 + 50 + gap-12, 200, 50, state, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Manga", keyboard_x + 680 + 30, 10 + 50 + gap-12, 200, 50, ThemeButtonText, Screen::normal);

  if(search_flags & (int)Category::ArtistCg) state = ThemeOptionSelected; else state = ThemeOptionUnselected;
  Screen::draw_button(keyboard_x + 680 + 30, 10 + 2*(50 + gap-12), 200, 50, state, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Artist Cg", keyboard_x + 680 + 30, 10 + 2*(50 + gap-12), 200, 50, ThemeButtonText, Screen::normal);

  if(search_flags & (int)Category::GameCg) state = ThemeOptionSelected; else state = ThemeOptionUnselected;
  Screen::draw_button(keyboard_x + 680 + 30, 10 + 3*(50 + gap-12), 200, 50, state, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Game Cg", keyboard_x + 680 + 30, 10 + 3*(50 + gap-12), 200, 50, ThemeButtonText, Screen::normal);

  if(search_flags & (int)Category::Western) state = ThemeOptionSelected; else state = ThemeOptionUnselected;
  Screen::draw_button(keyboard_x + 680 + 30, 10 + 4*(50 + gap-12), 200, 50, state, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Western", keyboard_x + 680 + 30, 10 + 4*(50 + gap-12), 200, 50, ThemeButtonText, Screen::normal);

  if(search_flags & (int)Category::NonH) state = ThemeOptionSelected; else state = ThemeOptionUnselected;
  Screen::draw_button(keyboard_x + 680 + 250, 10, 200, 50, state, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Non-H", keyboard_x + 680 + 250, 10, 200, 50, ThemeButtonText, Screen::normal);

  if(search_flags & (int)Category::ImageSet) state = ThemeOptionSelected; else state = ThemeOptionUnselected;
  Screen::draw_button(keyboard_x + 680 + 250, 10 + 50 + gap-12, 200, 50, state, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Image Set", keyboard_x + 680 + 250, 10 + 50 + gap-12, 200, 50, ThemeButtonText, Screen::normal);

  if(search_flags & (int)Category::Cosplay) state = ThemeOptionSelected; else state = ThemeOptionUnselected;
  Screen::draw_button(keyboard_x + 680 + 250, 10 + 2*(50 + gap-12), 200, 50, state, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Cosplay", keyboard_x + 680 + 250, 10 + 2*(50 + gap-12), 200, 50, ThemeButtonText, Screen::normal);

  if(search_flags & (int)Category::AsianPorn) state = ThemeOptionSelected; else state = ThemeOptionUnselected;
  Screen::draw_button(keyboard_x + 680 + 250, 10 + 3*(50 + gap-12), 200, 50, state, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Asian Porn", keyboard_x + 680 + 250, 10 + 3*(50 + gap-12), 200, 50, ThemeButtonText, Screen::normal);

  if(search_flags & (int)Category::Misc) state = ThemeOptionSelected; else state = ThemeOptionUnselected;
  Screen::draw_button(keyboard_x + 680 + 250, 10 + 4*(50 + gap-12), 200, 50, state, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Misc", keyboard_x + 680 + 250, 10 + 4*(50 + gap-12), 200, 50, ThemeButtonText, Screen::normal);


}
