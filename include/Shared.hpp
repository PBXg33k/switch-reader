#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <map>

enum class HandlerEnum { Browser,
                     Search,
                     Gallery,
                     Preview,
                     Settings,
                     Keyboard };

enum class Category : int { Doujinshi=1,
                      Manga=2,
                      ArtistCg=4,
                      GameCg=8,
                      Western=16,
                      NonH=32,
                      ImageSet=64,
                      Cosplay=128,
                      AsianPorn=256,
                      Misc=512 };

struct FloatPoint {
  float x;
  float y;
};

class Handler {
  public:
    static void set_touch();
    static HandlerEnum on_event(int val);
    static void render();
    static void scroll(float dx, float dy);
    static void gesture(SDL_Event e);
    static void finger_down(SDL_Event e);
    static void finger_up(SDL_Event e);

    static std::map<SDL_FingerID, FloatPoint> fingerTouches;
};

class Shared{
  public:
    static const int joy_val[24];
    static HandlerEnum do_event(HandlerEnum handler, int val);
    static const int KeyboardReturn = 9999;
};

extern std::string ApiProxy;
extern std::string UserURL;

extern SDL_Color ThemeBG;
extern SDL_Color ThemeBG_Trans;
extern SDL_Color ThemeButton;
extern SDL_Color ThemeButtonQuit;
extern SDL_Color ThemeButtonText;
extern SDL_Color ThemeButtonBorder;
extern SDL_Color ThemePanelDark;
extern SDL_Color ThemePanelLight;
extern SDL_Color ThemeText;
extern SDL_Color ThemePanelSelectedDark;
extern SDL_Color ThemePanelSelectedLight;
extern SDL_Color ThemeOptionSelected;
extern SDL_Color ThemeOptionUnselected;

extern SDL_Color COLOR_WHITE;
extern SDL_Color COLOR_BLACK;
extern SDL_Color COLOR_LIGHTGRAY;
extern SDL_Color COLOR_GRAY;
extern SDL_Color COLOR_DARKRED;
extern SDL_Color COLOR_RED;
extern SDL_Color COLOR_PALEISHYELLOW;
extern SDL_Color COLOR_PALEISHRED;
extern SDL_Color COLOR_PALERED;
extern SDL_Color COLOR_PALEYELLOW;
extern SDL_Color COLOR_PALEYELLOW_TRANS;
extern SDL_Color COLOR_PALEWHITE;
extern SDL_Color COLOR_VLIGHTGRAY;

extern SDL_Color COLOR_PALEBLACK;
extern SDL_Color COLOR_SHADEDBLACK;
extern SDL_Color COLOR_SHADEDBLACK_TRANS;
extern SDL_Color COLOR_YELLOWWHITE;
extern SDL_Color COLOR_TRUEGRAY;
extern SDL_Color COLOR_DARKISH;
extern SDL_Color COLOR_SUPERGRAY;

static const int screen_width = 1280;
static const int screen_height = 720;