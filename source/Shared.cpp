#include "Shared.hpp"

SDL_Color COLOR_WHITE = {255,255,255};
SDL_Color COLOR_BLACK = {0,0,0};
SDL_Color COLOR_LIGHTGRAY = {50,50,50};
SDL_Color COLOR_GRAY = {20,20,20};
SDL_Color COLOR_DARKRED = {50, 0, 0};
SDL_Color COLOR_RED = {128, 13, 18};
SDL_Color COLOR_PALEISHYELLOW = {200, 180, 185};
SDL_Color COLOR_PALEISHRED = {180, 105, 110};
SDL_Color COLOR_PALERED = {210, 155, 160};
SDL_Color COLOR_PALEYELLOW = {224, 222, 211};
SDL_Color COLOR_PALEWHITE = {237, 235, 223};
SDL_Color COLOR_VLIGHTGRAY = {150, 150, 150};

SDL_Color ThemeBG = COLOR_PALEYELLOW;
SDL_Color ThemeButton = COLOR_PALEWHITE;
SDL_Color ThemeButtonQuit = COLOR_RED;
SDL_Color ThemeButtonText = COLOR_LIGHTGRAY;
SDL_Color ThemeButtonBorder = COLOR_LIGHTGRAY;
SDL_Color ThemePanelDark = COLOR_PALEISHYELLOW;
SDL_Color ThemePanelLight = COLOR_PALEWHITE;
SDL_Color ThemePanelSelectedDark = COLOR_PALEISHRED;
SDL_Color ThemePanelSelectedLight = COLOR_PALERED;
SDL_Color ThemeText = COLOR_GRAY;
SDL_Color ThemeOptionSelected = COLOR_PALEWHITE;
SDL_Color ThemeOptionUnselected = COLOR_VLIGHTGRAY;

static void set_theme(int theme){
  switch(theme){
    // Light
    case 0:
      ThemeBG = COLOR_PALEYELLOW;
      ThemeButton = COLOR_PALEWHITE;
      ThemeButtonQuit = COLOR_RED;
      ThemeButtonText = COLOR_LIGHTGRAY;
      ThemeButtonBorder = COLOR_LIGHTGRAY;
      ThemePanelDark = COLOR_PALEISHYELLOW;
  }
}
