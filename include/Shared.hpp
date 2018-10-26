#ifndef SHARED_HPP
#define SHARED_HPP
#include <SDL2/SDL.h>
enum class Handler { Browser, Search, Gallery, };

extern SDL_Color ThemeBG;
extern SDL_Color ThemeButton;
extern SDL_Color ThemeButtonQuit;
extern SDL_Color ThemeButtonText;
extern SDL_Color ThemePanelDark;
extern SDL_Color ThemePanelLight;
extern SDL_Color ThemeText;
extern SDL_Color ThemePanelSelectedDark;
extern SDL_Color ThemePanelSelectedLight;

extern SDL_Color COLOR_WHITE;
extern SDL_Color COLOR_BLACK;
extern SDL_Color COLOR_LIGHTGRAY;
extern SDL_Color COLOR_GRAY;
extern SDL_Color COLOR_DARKRED;
extern SDL_Color COLOR_RED;
extern SDL_Color COLOR_PALERED;
extern SDL_Color COLOR_PALEYELLOW;
extern SDL_Color COLOR_PALEISHYELLOW;
extern SDL_Color COLOR_PALEYELLOW;
extern SDL_Color COLOR_PALEWHITE;

static const int screen_width = 1280;
static const int screen_height = 720;
#endif
