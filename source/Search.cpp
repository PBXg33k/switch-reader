#include "Search.hpp"
#include "Ui.hpp"
#include "TouchManager.hpp"
#include "HSearch.hpp"

int SearchBrowser::box_size = 75;
int SearchBrowser::gap = 25;
int SearchBrowser::active_elem = 0;
std::string SearchBrowser::search_str;
std::vector<char> alphabet = { 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'Z', 'X', 'C', 'V', 'B', 'N', 'M' };

void SearchBrowser::set_touch(){
  int letter = 0;
  int i;

  TouchManager::clear();

  for(i = 0; i < 10; i++){
    TouchManager::add_bounds(40 + (i * (box_size + gap)), (screen_height - (40 + 3*(box_size + gap))), box_size, box_size, letter);
    letter++;
  }

  // Middle row
  for(i = 0; i < 9; i++){
    TouchManager::add_bounds((40 + (box_size / 2) + ((box_size + gap) * i)), (screen_height - (40 + 2*(box_size + gap))), box_size, box_size, letter);
    letter++;
  }

  // Bottom row
  for(i = 0; i < 7; i++){
    TouchManager::add_bounds((40 + box_size) + ((box_size + gap) * i), (screen_height - (40 + box_size + gap)), box_size, box_size, letter);
    letter++;
  }
  // Backspace
  TouchManager::add_bounds(1280 - (40 + (box_size*2)), (screen_height - (40 + 3*(box_size + gap))), box_size * 2, box_size, 27);

  TouchManager::add_bounds(screen_width - 75, 0, 75, 75, 101);

  // TODO Set up fields
}

Handler SearchBrowser::on_event(int val){

  // A-Z
  if(val >= 0 && val < 26){
    search_str.append(std::string(1,alphabet[val]));
  }

  // Special characters
  switch(val){
    // Backspace
    case 27:
      if(!search_str.empty())
        search_str.resize(search_str.size() - 1);
      break;
    default:
      break;
  }

  // > 50 leaves 27-49 for special characters, if desired later
  if(val > 50){
    // TODO Field press input
  }

  if(val == 101){
    // Clear current search
    Browser::clear();

    // Do search
    std::vector<Entry> result = HSearch::search_keywords(search_str.c_str(), 25);
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

  // Top row keys
  for(i = 0; i < 10; i++){
    Screen::draw_button(40 + (i * (box_size + gap)), (screen_height - (40 + 3*(box_size + gap))), box_size, box_size, ThemeButtonBorder, ThemeButton, 5);
    Screen::draw_text_centered(std::string(1, alphabet[letter]), 40 + (i * (box_size + gap)), (screen_height - (40 + 3*(box_size + gap))), box_size, box_size, ThemeButtonText, Screen::normal);
    letter++;
  }

  // Middle row
  for(i = 0; i < 9; i++){
    Screen::draw_button((40 + (box_size / 2) + ((box_size + gap) * i)), (screen_height - (40 + 2*(box_size + gap))), box_size, box_size, ThemeButtonBorder, ThemeButton, 5);
    Screen::draw_text_centered(std::string(1, alphabet[letter]), (40 + (box_size / 2) + ((box_size + gap) * i)), (screen_height - (40 + 2*(box_size + gap))), box_size, box_size, ThemeButtonText, Screen::normal);
    letter++;
  }

  // Bottom row
  for(i = 0; i < 7; i++){
    Screen::draw_button((40 + box_size) + ((box_size + gap) * i), (screen_height - (40 + box_size + gap)), box_size, box_size, ThemeButtonBorder, ThemeButton, 5);
    Screen::draw_text_centered(std::string(1, alphabet[letter]), (40 + box_size) + ((box_size + gap) * i), (screen_height - (40 + box_size + gap)), box_size, box_size, ThemeButtonText, Screen::normal);
    letter++;
  }
  // Backspace
  Screen::draw_button(1280 - (40 + (box_size*2)), (screen_height - (40 + 3*(box_size + gap))), box_size * 2, box_size, ThemeButtonBorder, ThemeButton, 5);
  Screen::draw_text_centered("Backspace", 1280 - (40 + (box_size*2)), (screen_height - (40 + 3*(box_size + gap))), box_size * 2, box_size, ThemeButtonText, Screen::normal);

  // TODO Render fields

  // Render search string
  Screen::draw_button(300, 100, 680, 120, ThemeButtonBorder, ThemeButton, 6);
  if(!search_str.empty())
    Screen::draw_text_centered(search_str, 300, 100, 680, 120, ThemeButtonText, Screen::large);

  // Back button
  Screen::draw_rect(screen_width - 75, 0, 75, 75, ThemeButtonQuit);

  // TODO Render buttons
}
