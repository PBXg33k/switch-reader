#include "Keyboard.hpp"
#include "Touch.hpp"
#include "Ui.hpp"

int Keyboard::box_size = 80;
int Keyboard::gap = 15;
int Keyboard::keyboard_x = 40;
int Keyboard::keyboard_y = screen_height - 40;
int Keyboard::active_elem = 0;
int Keyboard::caps_lock = 0;

std::string Keyboard::text;
static Handler handler = Handler::Browser;

std::vector<char> alphabet = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
      'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
      'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'',
      'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/' };
std::vector<char> altAlphabet = { '!', '"', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
      'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 
      'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '@',
      'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?' };

void Keyboard::setup(Handler handle, std::string start_str){
  handler = handle;
  text = start_str;
}

void Keyboard::set_touch(){
  int letter = 0;
  int i;

  TouchManager::clear();

  // Number row
  for(i = 0; i < 12; i++){
    TouchManager::add_bounds(keyboard_x + (i * (box_size + gap)), keyboard_y - 5*(box_size + gap), box_size, box_size, letter);
    letter++;
  }

  // Top row
  for(i = 0; i < 12; i++){
    TouchManager::add_bounds(keyboard_x + (box_size / 2) + (i * (box_size + gap)), keyboard_y - 4*(box_size + gap), box_size, box_size, letter);
    letter++;
  }

  // Middle row
  for(i = 0; i < 11; i++){
    TouchManager::add_bounds(keyboard_x + box_size + ((box_size + gap) * i), keyboard_y - 3*(box_size + gap), box_size, box_size, letter);
    letter++;
  }

  // Bottom row
  for(i = 0; i < 10; i++){
    TouchManager::add_bounds(keyboard_x + (box_size*1.5) + ((box_size + gap) * i), keyboard_y - 2*(box_size + gap), box_size, box_size, letter);
    letter++;
  }
  // Backspace
  TouchManager::add_bounds(keyboard_x + (box_size*1.5) + ((box_size + gap) * 10), keyboard_y - 2*(box_size + gap), box_size * 1.5, box_size, 50);
  // Caps Lock
  TouchManager::add_bounds(keyboard_x, keyboard_y - 2*(box_size + gap), box_size * 1.5 - gap, box_size, 51);
  // Space
  TouchManager::add_bounds(keyboard_x + (box_size*2), keyboard_y - (box_size + gap), box_size * 10, box_size, 52);

  // Return to Browser
  TouchManager::add_bounds(screen_width - 75, 0, 75, 75, 101);
}

Handler Keyboard::on_event(int val){
  // Number row, A-Z and , and .
  if(val >= 0 && val < 50){
    if(caps_lock)
      text.append(std::string(1,altAlphabet[val]));
    else
      text.append(std::string(1,alphabet[val]));
  }

  // Special characters
  switch(val){
    // Backspace
    case 50:
      if(!text.empty())
        text.resize(text.size() - 1);
      break;
    // Caps lock
    case 51:
      caps_lock = !caps_lock;
      break;
    // Space
    case 52:
      text.append(" ");
      break;
    default:
      break;
  }

  if(val == 101){
    Shared::do_event(handler, Shared::KeyboardReturn);
    return handler;
  }

  return Handler::Keyboard;
}

void Keyboard::render(){
  Screen::clear(ThemeBG);
  int i;
  int letter = 0;
  std::vector<char> alpha = alphabet;
  if(caps_lock)
    alpha = altAlphabet;

  // Number row keys
  for(i = 0; i < 12; i++){
    Screen::draw_button(keyboard_x + (i * (box_size + gap)), keyboard_y - 5*(box_size + gap), box_size, box_size, ThemeButton, ThemeButtonBorder, 5);
    Screen::draw_text_centered(std::string(1, alpha[letter]), keyboard_x + (i * (box_size + gap)), keyboard_y - 5*(box_size + gap), box_size, box_size, ThemeButtonText, Screen::large);
    letter++;
  }

  // Top row keys
  for(i = 0; i < 12; i++){
    Screen::draw_button(keyboard_x + (box_size / 2) + (i * (box_size + gap)), keyboard_y - 4*(box_size + gap), box_size, box_size, ThemeButton, ThemeButtonBorder, 5);
    Screen::draw_text_centered(std::string(1, alpha[letter]), keyboard_x + (box_size / 2) + (i * (box_size + gap)), keyboard_y - 4*(box_size + gap), box_size, box_size, ThemeButtonText, Screen::large);
    letter++;
  }

  // Middle row
  for(i = 0; i < 11; i++){
    Screen::draw_button(keyboard_x + box_size + ((box_size + gap) * i), keyboard_y - 3*(box_size + gap), box_size, box_size, ThemeButton, ThemeButtonBorder, 5);
    Screen::draw_text_centered(std::string(1, alpha[letter]), keyboard_x + box_size + ((box_size + gap) * i), keyboard_y - 3*(box_size + gap), box_size, box_size, ThemeButtonText, Screen::large);
    letter++;
  }

  // Bottom row
  for(i = 0; i < 10; i++){
    Screen::draw_button(keyboard_x + (box_size*1.5) + ((box_size + gap) * i), keyboard_y - 2*(box_size + gap), box_size, box_size, ThemeButton, ThemeButtonBorder, 5);
    Screen::draw_text_centered(std::string(1, alpha[letter]), keyboard_x + (box_size*1.5) + ((box_size + gap) * i), keyboard_y - 2*(box_size + gap), box_size, box_size, ThemeButtonText, Screen::large);
    letter++;
  }
  // Backspace
  Screen::draw_button(keyboard_x + (box_size*1.5) + ((box_size + gap) * 10), keyboard_y - 2*(box_size + gap), box_size * 1.5, box_size, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text_centered("Bksp", keyboard_x + (box_size*1.5) + ((box_size + gap) * 10), keyboard_y - 2*(box_size + gap), box_size * 1.5, box_size, ThemeButtonText, Screen::large);

  // Caps Lock
  Screen::draw_button(keyboard_x, keyboard_y - 2*(box_size + gap), box_size * 1.5 - gap, box_size, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text_centered("Shift", keyboard_x, keyboard_y - 2*(box_size + gap), box_size * 1.5 - gap, box_size, ThemeButtonText, Screen::large);

  // Space
  Screen::draw_button(keyboard_x + (box_size*2), keyboard_y - (box_size + gap), box_size * 10, box_size, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text_centered("Space", keyboard_x + (box_size*2), keyboard_y - (box_size + gap), box_size * 10, box_size, ThemeButtonText, Screen::large);

  // TODO Render fields

  // Render search string
  Screen::draw_button(keyboard_x, 60, 680, 100, ThemeButton, ThemeButtonBorder, 6);
  if(!text.empty())
    Screen::draw_text_centered(text, keyboard_x, 60, 680, 100, ThemeButtonText, Screen::large);

  // Back button
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);
}