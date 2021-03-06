#include "Shared.hpp"

std::string ApiProxy;
std::string UserURL = "https://forums.e-hentai.org/index.php?act=UserCP&CODE=00";

SDL_Color COLOR_WHITE = {255,255,255,255};
SDL_Color COLOR_BLACK = {0,0,0,255};
SDL_Color COLOR_LIGHTGRAY = {50,50,50,255};
SDL_Color COLOR_GRAY = {20,20,20,255};
SDL_Color COLOR_DARKRED = {50, 0, 0,255};
SDL_Color COLOR_RED = {128, 13, 18,255};
SDL_Color COLOR_PALEISHYELLOW = {200, 180, 185,255};
SDL_Color COLOR_PALEISHRED = {180, 105, 110,255};
SDL_Color COLOR_PALERED = {210, 155, 160,255};
SDL_Color COLOR_PALEYELLOW = {224, 222, 211,255};
SDL_Color COLOR_PALEYELLOW_TRANS = {224, 222, 211, 80};
SDL_Color COLOR_PALEWHITE = {237, 235, 223,255};
SDL_Color COLOR_VLIGHTGRAY = {150, 150, 150,255};

SDL_Color COLOR_PALEBLACK = {79, 83, 91,255};
SDL_Color COLOR_SHADEDBLACK = {52, 53, 59,255};
SDL_Color COLOR_SHADEDBLACK_TRANS = {52, 53, 59, 80};
SDL_Color COLOR_YELLOWWHITE = {241, 235, 206,255};
SDL_Color COLOR_TRUEGRAY = {141, 141, 141,255};
SDL_Color COLOR_DARKISH = {64, 69, 75,255};
SDL_Color COLOR_SUPERGRAY = {221, 221, 221,255};

SDL_Color ThemeBG = COLOR_PALEYELLOW;
SDL_Color ThemeBG_Trans = COLOR_PALEYELLOW_TRANS;
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

const int Shared::joy_val[24] = {102, 101, 110, 111, 0, 0, 0, 0, 103, 104, 0, 0, 123, 120, 121, 122, 123, 120, 121, 122, 0, 0, 0, 0};

std::map<SDL_FingerID, FloatPoint> Handler::fingerTouches;
void Handler::set_touch() {};
HandlerEnum Handler::on_event(int val) { return HandlerEnum::Browser; };
void Handler::render() {};
void Handler::scroll(float dx, float dy){};
void Handler::gesture(SDL_Event e) {};
void Handler::finger_down(SDL_Event e) {};
void Handler::finger_up(SDL_Event e) {};