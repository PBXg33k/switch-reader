#include "Dialog.hpp"
#include "Ui.hpp"
#include "Touch.hpp"
#include "Shared.hpp"

#define gap 15

int Dialog::get_response(std::string message, std::vector<std::string> choices){

  SDL_Event event;
  int res = -1;
  int width = (screen_width - (gap*5)) / 3;
  int height = ((screen_height - 150) - (gap * 5)) / 3;
  int start = 0;

  // Set up touches
  TouchManager touch;
  // Quit button
  touch.add_bounds(screen_width - 75, 0, 75, 75, 101);
  // Touch 3 in a line
  for(int row = 0; row*3 < (int) choices.size(); row++){
    int numInRow = 3;
    if(choices.size() - (row * 3) < 3)
      numInRow = choices.size() - (row * 3);

    start = (screen_width / 2) - ((width * numInRow) / 2) - (gap * (numInRow - 1));
    switch(numInRow){
      case 3:
        touch.add_bounds(start + 2*(width + gap), 150 + (row * height) + ((row+1)*gap), width, height, (row*3) + 2);
      case 2:
        touch.add_bounds(start + width + gap, 150 + (row * height) + ((row+1)*gap), width, height, (row*3) + 1);
      case 1:
        touch.add_bounds(start, 150 + (row * height) + ((row+1)*gap), width, height, row*3);
        break;
      default:
        break;
    }
  }


  while(res < 0){
    // Begin render
    Screen::clear(ThemeBG);
    // Question
    Screen::draw_text_centered(message, 0, 0, screen_width, 150, ThemeText, Screen::header);
    // Quit button
    Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);
    // Render 3 in a line

    for(int row = 0; row*3 < (int) choices.size(); row++){
      // Get upto 3 points in row
      std::string first = choices[row*3];
      std::string second;
      std::string third;

      for(int row = 0; row*3 < (int) choices.size(); row++){
        int numInRow = 3;
        if(choices.size() - (row * 3) < 3)
          numInRow = choices.size() - (row * 3);

        start = (screen_width / 2) - ((width * numInRow) / 2) - (gap * (numInRow - 1) / 2);

        switch(numInRow){
          case 3:
            Screen::draw_button(start + 2*(width + gap), 150 + (row * height) + ((row+1)*gap), width, height, ThemeButton, ThemeButtonBorder, 5);
            Screen::draw_text_centered(choices[row*3 + 2], start + 2*(width + gap), 150 + (row * height) + ((row+1)*gap), width, height, ThemeText, Screen::large);
          case 2:
            Screen::draw_button(start + width + gap, 150 + (row * height) + ((row+1)*gap), width, height, ThemeButton, ThemeButtonBorder, 5);
            Screen::draw_text_centered(choices[row*3 + 1], start + width + gap, 150 + (row * height) + ((row+1)*gap), width, height, ThemeText, Screen::large);
          case 1:
            Screen::draw_button(start, 150 + (row * height) + ((row+1)*gap), width, height, ThemeButton, ThemeButtonBorder, 5);
            Screen::draw_text_centered(choices[row*3], start, 150 + (row * height) + ((row+1)*gap), width, height, ThemeText, Screen::large);
            break;
          default:
            break;
        }
      }
    }

    Screen::render();

    // Check inputs
    while(SDL_PollEvent(&event)){
      int val = -1;

      switch(event.type){
        case SDL_JOYBUTTONDOWN:
          val = Shared::joy_val[event.jbutton.button];
          break;
        case SDL_FINGERDOWN:
          val = touch.get_value(event.tfinger.x*screen_width, event.tfinger.y*screen_height);
          break;
        default:
          break;
      }

      res = val;
    }

  }

  return res;
}