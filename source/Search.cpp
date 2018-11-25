#include "Search.hpp"
#include "Keyboard.hpp"
#include "Ui.hpp"
#include "Touch.hpp"
#include "HSearch.hpp"
#include "Browser.hpp"
#include "Api.hpp"
#include "Config.hpp"
#include <tgmath.h>

#define starScale 2

int SearchBrowser::box_size = 75;
int SearchBrowser::gap = 20;
int SearchBrowser::keyboard_x = 40;
int SearchBrowser::keyboard_y = screen_height - 40;
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

  // Star Ratings
  // 1-5
  for(int i = 0; i < 5; i++)
    TouchManager::add_bounds(keyboard_x + (31 * i * starScale), 460, 62, 30 * starScale, 150 + i);

  // Public or Favourites
  TouchManager::add_bounds(keyboard_x + 680 + 30, 10 + 5*(50 + gap-12), 420, 100, 111);

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
    int flags = stoi(ConfigManager::get_value("categories"));
    if(flags & flag)
      flags = flags & ~flag;
    else
      flags = flags | flag;

    ConfigManager::set_pair("categories", std::to_string(flags));
  }

  if(val == 101){
    // Clear current search
    Browser::clear();

    // Do search
    HSearch::search_keywords(search_str.c_str(), 25, stoi(ConfigManager::get_value("categories")));

    // Go back to Browser
    Browser::set_touch();
    return Handler::Browser;
  }

  // Open keyboard
  if(val == 102){
    field = 0;
    Keyboard::setup(Handler::Search, search_str);
    Keyboard::set_touch();
    return Handler::Keyboard;
  }
  
  // Open lang keyboard
  if(val == 110){
    field = 1;
    Keyboard::setup(Handler::Search, ConfigManager::get_value("lang"));
    Keyboard::set_touch();
    return Handler::Keyboard;
  }
  // Turn stars on/set
  if(val >= 150 && val < 155){
    ConfigManager::set_pair("stars", std::to_string((val - 150) + 1));
    printf("Set %d stars\n", (val-150) + 1);
  }

  // Toggle favourites
  if(val == 111){
    if(ConfigManager::get_value("search") == "Public")
      ConfigManager::set_pair("search", "Favourites");
    else
      ConfigManager::set_pair("search", "Public");
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

  return Handler::Search;
}

void SearchBrowser::render(){
  Screen::clear(ThemeBG);

  // Render search string
  Screen::draw_text("Search", keyboard_x, 15, ThemeText, Screen::large);
  Screen::draw_button(keyboard_x, 60, 680, 100, ThemeButton, ThemeButtonBorder, 6);
  Screen::draw_text_centered(search_str, keyboard_x, 60, 680, 100, ThemeButtonText, Screen::large);

  // Render language selection
  Screen::draw_text("Language", keyboard_x, 215, ThemeText, Screen::large);
  Screen::draw_button(keyboard_x, 260, 680, 100, ThemeButton, ThemeButtonBorder, 6);
  Screen::draw_text_centered(ConfigManager::get_value("lang"), keyboard_x, 260, 680, 100, ThemeButtonText, Screen::large);

  Screen::draw_text("Min. Star Rating", keyboard_x, 410, ThemeText, Screen::large);
  // Empty stars
  Screen::draw_partial(keyboard_x, 460, 1, 1, Screen::s_stars_off, starScale);
  // Filled stars
  Screen::draw_partial(keyboard_x, 460, stof(ConfigManager::get_value("stars")) / 5, 1, Screen::s_stars, starScale);

  // Public or Favourites
  Screen::draw_button(keyboard_x + 680 + 30, 10 + 5*(50 + gap-12), 420, 100, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text_centered(ConfigManager::get_value("search"), keyboard_x + 680 + 30, 10 + 5*(50 + gap-12), 420, 100, ThemeButtonText, Screen::large);

  // Back button
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);

  SDL_Color state;

  int search_flags = stoi(ConfigManager::get_value("categories"));

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
