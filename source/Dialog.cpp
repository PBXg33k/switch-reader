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

int Dialog::get_number(std::string message, int rot, int max, int start){
  // Set up touch
  int block_size = 150;
  int cur_num = start;
  int val = -1;
  SDL_Event event;
  TouchManager touch;

  int start_x = (screen_width/2) - (((block_size * 3) + (gap * 2)) / 2);
  int start_y = (screen_height/2) - (((block_size * 3) + (gap * 2)) / 2);
  int i, j, x, y;

  int num = 1;
  for(i = 0; i < 3; i++){
    y = start_y + (i * (block_size + gap)); 
    for(j = 0; j < 3; j++){
      x = start_x + (j * (block_size + gap));
      touch.add_bounds(x, y, block_size, block_size, num);
      num++;
    }
  }

  // Quit button
  touch.add_bounds(screen_width - 75, 0, 75, 75, 101);

  while(1){
    // Render numpad
    int num = 1;
    for(i = 0; i < 3; i++){
      y = start_y + (i * (block_size + gap)); 
      for(j = 0; j < 3; j++){
        x = start_x + (j * (block_size + gap));
        Screen::draw_button(x, y, block_size, block_size, ThemeButton, ThemeButtonBorder, 5);
        Screen::draw_text_centered(std::to_string(num), x, y, block_size, block_size, ThemeButtonText, Screen::header);
        num++;
      }
    }

    // Quit button
    Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);

    // Render number
    Screen::draw_text_centered(std::to_string(cur_num), 0, 0, screen_width, 112, ThemeText, Screen::header);
    
    // Events
    while(SDL_PollEvent(&event)){
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
    }

    return cur_num;
  }
}

bool Dialog::get_bool(std::string message){
  printf("Getting response\n");
  int res = Dialog::get_response(message, {"No", "Yes"});
  printf("Response was %d\n", res);

  if(res == 1)
    return true;
  else
    return false;
}