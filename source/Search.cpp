#include "Search.hpp"
#include "Ui.hpp"
#include "TouchManager.hpp"
#include "HSearch.hpp"
#include <tgmath.h>

int SearchBrowser::box_size = 75;
int SearchBrowser::gap = 20;
int SearchBrowser::keyboard_x = 40;
int SearchBrowser::keyboard_y = screen_height - 40;
int SearchBrowser::active_elem = 0;
int SearchBrowser::caps_lock = 0;

std::string SearchBrowser::search_str;
int SearchBrowser::search_flags = (int)Category::NonH;

std::vector<char> alphabet = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '=', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.' };
std::vector<char> altAlphabet = { '!', '"', '#', '$', '%', '^', '&', '*', '(', ')', '+', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>' };

void SearchBrowser::set_touch(){
  int letter = 0;
  int i;

  TouchManager::clear();

  // Number row
  for(i = 0; i < 11; i++){
    TouchManager::add_bounds(keyboard_x + (i * (box_size + gap)), keyboard_y - 4*(box_size + gap), box_size, box_size, letter);
    letter++;
  }

  // Top row
  for(i = 0; i < 10; i++){
    TouchManager::add_bounds(keyboard_x + (box_size / 2) + (i * (box_size + gap)), keyboard_y - 3*(box_size + gap), box_size, box_size, letter);
    letter++;
  }

  // Middle row
  for(i = 0; i < 9; i++){
    TouchManager::add_bounds(keyboard_x + box_size + ((box_size + gap) * i), keyboard_y - 2*(box_size + gap), box_size, box_size, letter);
    letter++;
  }

  // Bottom row
  for(i = 0; i < 9; i++){
    TouchManager::add_bounds(keyboard_x + (box_size*1.5) + ((box_size + gap) * i), keyboard_y - (box_size + gap), box_size, box_size, letter);
    letter++;
  }
  // Backspace
  TouchManager::add_bounds(1280 - (40 + (box_size*2)), (screen_height - (40 + 4*(box_size + gap))), box_size * 2, box_size, 40);
  // Caps Lock
  TouchManager::add_bounds(1280 - (40 + (box_size*2)), (screen_height - (40 + 3*(box_size + gap))), box_size * 2, box_size, 41);
  // Space
  TouchManager::add_bounds(1280 - (40 + (box_size*2)), (screen_height - (40 + 2*(box_size + gap))), box_size * 2, box_size, 42);

  // Return to Browser
  TouchManager::add_bounds(screen_width - 75, 0, 75, 75, 101);


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

  // Number row, A-Z and , and .
  if(val >= 0 && val < 40){
    search_str.append(std::string(1,alphabet[val]));
  }

  // Special characters
  switch(val){
    // Backspace
    case 40:
      if(!search_str.empty())
        search_str.resize(search_str.size() - 1);
      break;
    // Caps lock
    case 41:
      caps_lock = !caps_lock;
      break;
    // Space
    case 42:
      search_str.append(" ");
      break;
    default:
      break;
  }

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
    std::vector<Entry> result = HSearch::search_keywords(search_str.c_str(), 25, (int)search_flags);
    for(auto entry : result){
      printf("Returned %s\n", entry.url.c_str());
      Browser::add_entry(entry);
    }

    // Go back to Browser
    Browser::set_touch();
    return Handler::Browser;
  }
  return Handler::Search;
}

void SearchBrowser::render(){
  Screen::clear(ThemeBG);
  int i;
  int letter = 0;
  std::vector<char> alpha = alphabet;
  if(caps_lock)
    alpha = altAlphabet;

  // Number row keys
  for(i = 0; i < 11; i++){
    Screen::draw_button(keyboard_x + (i * (box_size + gap)), keyboard_y - 4*(box_size + gap), box_size, box_size, ThemeButton, ThemeButtonBorder, 5);
    Screen::draw_text_centered(std::string(1, alpha[letter]), keyboard_x + (i * (box_size + gap)), keyboard_y - 4*(box_size + gap), box_size, box_size, ThemeButtonText, Screen::normal);
    letter++;
  }

  // Top row keys
  for(i = 0; i < 10; i++){
    Screen::draw_button(keyboard_x + (box_size / 2) + (i * (box_size + gap)), keyboard_y - 3*(box_size + gap), box_size, box_size, ThemeButton, ThemeButtonBorder, 5);
    Screen::draw_text_centered(std::string(1, alpha[letter]), keyboard_x + (box_size / 2) + (i * (box_size + gap)), keyboard_y - 3*(box_size + gap), box_size, box_size, ThemeButtonText, Screen::normal);
    letter++;
  }

  // Middle row
  for(i = 0; i < 9; i++){
    Screen::draw_button(keyboard_x + box_size + ((box_size + gap) * i), keyboard_y - 2*(box_size + gap), box_size, box_size, ThemeButton, ThemeButtonBorder, 5);
    Screen::draw_text_centered(std::string(1, alpha[letter]), keyboard_x + box_size + ((box_size + gap) * i), keyboard_y - 2*(box_size + gap), box_size, box_size, ThemeButtonText, Screen::normal);
    letter++;
  }

  // Bottom row
  for(i = 0; i < 9; i++){
    Screen::draw_button(keyboard_x + (box_size*1.5) + ((box_size + gap) * i), keyboard_y - (box_size + gap), box_size, box_size, ThemeButton, ThemeButtonBorder, 5);
    Screen::draw_text_centered(std::string(1, alpha[letter]), keyboard_x + (box_size*1.5) + ((box_size + gap) * i), keyboard_y - (box_size + gap), box_size, box_size, ThemeButtonText, Screen::normal);
    letter++;
  }
  // Backspace
  Screen::draw_button(1280 - (40 + (box_size*2)), (screen_height - (40 + 4*(box_size + gap))), box_size * 2, box_size, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text_centered("Backspace", 1280 - (40 + (box_size*2)), (screen_height - (40 + 4*(box_size + gap))), box_size * 2, box_size, ThemeButtonText, Screen::normal);

  // Caps Lock
  Screen::draw_button(1280 - (40 + (box_size*2)), (screen_height - (40 + 3*(box_size + gap))), box_size * 2, box_size, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text_centered("Caps Lock", 1280 - (40 + (box_size*2)), (screen_height - (40 + 3*(box_size + gap))), box_size * 2, box_size, ThemeButtonText, Screen::normal);

  // Space
  Screen::draw_button(1280 - (40 + (box_size*2)), (screen_height - (40 + 2*(box_size + gap))), box_size * 2, box_size, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text_centered("Space", 1280 - (40 + (box_size*2)), (screen_height - (40 + 2*(box_size + gap))), box_size * 2, box_size, ThemeButtonText, Screen::normal);

  // TODO Render fields

  // Render search string
  Screen::draw_button(keyboard_x, 110, 680, 100, ThemeButton, ThemeButtonBorder, 6);
  if(!search_str.empty())
    Screen::draw_text_centered(search_str, 110, 100, 680, 120, ThemeButtonText, Screen::large);

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
