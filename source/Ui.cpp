#include "Ui.hpp"
#include "Shared.hpp"

static SDL_Texture* s_refresh;
SDL_Texture* Screen::s_stars;
SDL_Texture* Screen::s_loading;
SDL_Texture* Screen::s_stars_off;
static PlFontData stdFont, extFont;

static void load_fonts(){
  Screen::gallery_info = FC_CreateFont();
  Screen::normal = FC_CreateFont();
  Screen::large = FC_CreateFont();
  Screen::header = FC_CreateFont();

  Result rc;
  plInitialize();
  plGetSharedFontByType(&stdFont, PlSharedFontType_Standard);
  plGetSharedFontByType(&extFont, PlSharedFontType_NintendoExt);

  if(!R_FAILED(rc)){
    FC_LoadFont_RW(Screen::gallery_info, Screen::renderer, SDL_RWFromMem(stdFont.address, stdFont.size), SDL_RWFromMem(extFont.address, extFont.size), 1, 18, FC_MakeColor(0, 0, 0, 255), TTF_STYLE_NORMAL);
    FC_LoadFont_RW(Screen::normal, Screen::renderer, SDL_RWFromMem(stdFont.address, stdFont.size), SDL_RWFromMem(extFont.address, extFont.size), 1, 24, FC_MakeColor(0, 0, 0, 255), TTF_STYLE_NORMAL);
    FC_LoadFont_RW(Screen::large, Screen::renderer, SDL_RWFromMem(stdFont.address, stdFont.size), SDL_RWFromMem(extFont.address, extFont.size), 1, 32, FC_MakeColor(0, 0, 0, 255), TTF_STYLE_NORMAL);
    FC_LoadFont_RW(Screen::header, Screen::renderer, SDL_RWFromMem(stdFont.address, stdFont.size), SDL_RWFromMem(extFont.address, extFont.size), 1, 48, FC_MakeColor(0, 0, 0, 255), TTF_STYLE_NORMAL);
  } else {
    printf("Pl service error?\n");
  }
}

void Screen::init()
{
  printf("Initalizing Screen...\n");

  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

  // Set up windows
  Screen::window = SDL_CreateWindow("Image Test",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_FULLSCREEN);
  Screen::renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  Screen::surface = SDL_GetWindowSurface(Screen::window);
  SDL_SetRenderDrawBlendMode(Screen::renderer, SDL_BLENDMODE_BLEND);

  romfsInit();

  // Load perm images   static SDL_Texture* loading;
  SDL_Surface* surf = IMG_Load("romfs:/failed.png");
  s_refresh = SDL_CreateTextureFromSurface(Screen::renderer, surf);
  SDL_FreeSurface(surf);

  surf = IMG_Load("romfs:/stars.png");
  s_stars = SDL_CreateTextureFromSurface(Screen::renderer, surf);
  SDL_FreeSurface(surf);

  surf = IMG_Load("romfs:/loading.png");
  s_loading = SDL_CreateTextureFromSurface(Screen::renderer, surf);
  SDL_FreeSurface(surf);

  surf = IMG_Load("romfs:/stars_off.png");
  s_stars_off = SDL_CreateTextureFromSurface(Screen::renderer, surf);
  SDL_FreeSurface(surf);

  romfsExit();

  load_fonts();

  // Render loading screen
  clear(ThemeBG);
  draw_text_centered("Loading...", 0, 0, screen_width, screen_height, ThemeText, Screen::header);
  render();
}

void Screen::cleanup_texture(SDL_Texture* texture){
  if(texture != NULL){
    if(texture != s_refresh && texture != s_stars && texture != s_loading){
      SDL_DestroyTexture(texture);
    }
    texture = NULL;
  }
}

// Clean up SDL, free resources etc
void Screen::close()
{
  // Fonts
  FC_FreeFont(gallery_info);
  FC_FreeFont(normal);
  FC_FreeFont(large);
  FC_FreeFont(header);

  // Textures
  SDL_DestroyTexture(s_refresh);
  SDL_DestroyTexture(s_stars);
  SDL_DestroyTexture(s_loading);
  IMG_Quit();

  // Windows
  SDL_DestroyRenderer(Screen::renderer);
  SDL_DestroyWindow(Screen::window);
  SDL_Quit();

  plExit();
}

SDL_Texture* Screen::load_texture(char* image, size_t size){
  SDL_RWops *rw = SDL_RWFromMem(image, size);
  SDL_Surface *surf = IMG_Load_RW(rw, 1);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(Screen::renderer, surf);
  SDL_FreeSurface(surf);
  if(texture == NULL){
    printf("SDL_CreateTextureFromSurface Error - %s\n", SDL_GetError());
    return load_stored_texture(0);
  }
  return texture;
}

SDL_Texture* Screen::load_stored_texture(int id){
  switch(id){
    case 0:
      return s_refresh;
    case 1:
      return s_stars;
    case 2:
      return s_loading;
    default:
      break;
  }
  return NULL;
}

void Screen::render(){
  SDL_RenderPresent(Screen::renderer);
}

void fit_image(SDL_Texture* texture, int x, int y)
{
  // Maintain ratio and fill screen
  int w, h;
  SDL_QueryTexture(texture, NULL, NULL, &w, &h);
  float scaler = 720.0f / ((float)(h));

  SDL_Rect rect;
  rect.w = (int)(w * scaler);
  rect.h = (int)(h * scaler);
  rect.x = (1280/2) - (rect.w / 2);
  rect.y = 0;

  SDL_RenderCopy(Screen::renderer, texture, NULL, &rect);
}

// Draw image from memory
void Screen::draw_image_mem(SDL_Texture* texture, int x, int y){
  fit_image(texture, x, y);
}

void Screen::draw_adjusted_mem(SDL_Texture* texture, int x, int y, int maxw, int maxh, int rot){
  if(texture){
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    float scaler;
    SDL_Rect rect;

    if(!(rot % 2)){
      scaler = maxh / ((float)(h));
      rect.w = (int)(w * scaler);
      rect.h = (int)(h * scaler);
      rect.x = x;
      rect.y = y;

      scaler = maxw / ((float)(rect.w));
      if(scaler < 1){
        rect.w *= scaler;
        rect.h *= scaler;
      }
    } else {
      scaler = maxh / ((float)(w));
      rect.w = (int)(w * scaler);
      rect.h = (int)(h * scaler);
      rect.x = x;
      rect.y = y;

      scaler = maxw / ((float)(rect.h));
      if(scaler < 1){
        rect.w *= scaler;
        rect.h *= scaler;
      }
    }

    // Centre Image Vertically
    rect.y = y + ((maxh - rect.h)/2);

    // Centre Image Horizontally
    rect.x = x + ((maxw - rect.w)/2);

    SDL_RenderCopyEx(Screen::renderer, texture, NULL, &rect, rot * 90, NULL, SDL_FLIP_NONE);
  }
}

void Screen::clear(SDL_Color color){
  SDL_Rect rect = {0,0, screen_width, screen_height};
  SDL_SetRenderDrawColor(Screen::renderer, color.r, color.g, color.b, color.a);
  SDL_RenderFillRect(Screen::renderer, &rect);
}

void Screen::draw_rect(int x, int y, int w, int h, SDL_Color color){
  SDL_Rect rect = {x,y,w,h};
  SDL_SetRenderDrawColor(Screen::renderer, color.r, color.g, color.b, color.a);
  SDL_RenderFillRect(Screen::renderer, &rect);

}

void Screen::draw_partial(int x, int y, double percentW, double percentH, SDL_Texture* texture, float scale){
  if(texture){
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect rect;
    SDL_Rect pos;

    rect.x = 0;
    rect.y = 0;
    rect.w = percentW * w;
    rect.h = percentH * h;

    pos.x = x;
    pos.y = y;
    pos.w = rect.w * scale;
    pos.h = rect.h * scale;

    SDL_RenderCopy(Screen::renderer, texture, &rect, &pos);
  }
}

void Screen::draw_button(int x, int y, int w, int h, SDL_Color fore, SDL_Color back, int border){
  // Draw back layer (border)
  if(border > 0)
    draw_rect(x, y, w, h, back);

  // Draw front layer
  draw_rect(x+border, y+border, w-(border*2), h-(border*2), fore);
}

void draw_text_internal(std::string text, int x, int y, SDL_Color color, FC_Font* font){
  FC_DrawColor(font, Screen::renderer, x, y, color, text.c_str());
}

void Screen::draw_text_aligned(std::string text, int x, int y, int maxw, int maxh, FC_AlignEnum align, SDL_Color color, FC_Font* font){
  if(text.empty())
    return;

  Uint16 height = FC_GetHeight(font, text.c_str());
  y += ((maxh - height) / 2);

  FC_DrawBoxColor(font, Screen::renderer, SDL_Rect {x,y,maxw,maxh}, color, align, text.c_str());
}

void Screen::draw_text_centered(std::string text, int x, int y, int maxw, int maxh, SDL_Color color, FC_Font* font){
  draw_text_aligned(text, x, y, maxw, maxh, FC_ALIGN_CENTER, color, font);
}

void Screen::draw_text(std::string text, int x, int y, SDL_Color color, FC_Font* font)
{
  draw_text_internal(text, x, y, color, font);
}

void Screen::draw_text(std::string text, int x, int y, SDL_Color color) {
  draw_text_internal(text, x, y, color, normal);
}

void draw_circle(SDL_Point center, int radius, SDL_Color color)
{
    SDL_SetRenderDrawColor(Screen::renderer, color.r, color.g, color.b, color.a);
    for (int w = 0; w < radius * 2; w++)
    {
        for (int h = 0; h < radius * 2; h++)
        {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx*dx + dy*dy) <= (radius * radius))
            {
                SDL_RenderDrawPoint(Screen::renderer, center.x + dx, center.y + dy);
            }
        }
    }
}

void Screen::draw_pill(int x, int y, int w, int h, SDL_Color color){
  // Make h have a center
  if(h % 2 == 0)
    h += 1;

  int radius = h / 2;
  Screen::draw_rect(x + radius, y, w - (radius * 2), h, color);

  // Draw 2 circles
  draw_circle({x + radius, h / 2}, radius, color);
  draw_circle({x + w - radius, h / 2}, radius, color);
}
