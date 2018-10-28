#include "Ui.hpp"
#include "Shared.hpp"

static SDL_Texture* s_refresh;

void Screen::init()
{
  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
  TTF_Init();

  plInitialize();

  Result rc;
  rc = plGetSharedFontByType(&Screen::standardFontData, PlSharedFontType_Standard);
  if(!R_FAILED(rc)){
    Screen::gallery_info = TTF_OpenFontRW(SDL_RWFromMem(Screen::standardFontData.address, Screen::standardFontData.size), 1, 18);
    Screen::normal = TTF_OpenFontRW(SDL_RWFromMem(Screen::standardFontData.address, Screen::standardFontData.size), 1, 24);
    Screen::large = TTF_OpenFontRW(SDL_RWFromMem(Screen::standardFontData.address, Screen::standardFontData.size), 1, 30);
  }

  // Screen::gallery_info = TTF_OpenFont("Helvetica.ttf", 18);
  // Screen::normal = TTF_OpenFont("Helvetica.ttf", 24);

  // Set up windows
  Screen::window = SDL_CreateWindow("Image Test",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, 0);
  Screen::renderer = SDL_CreateRenderer(window, -1, 0);
  Screen::surface = SDL_GetWindowSurface(Screen::window);

  // Load perm images
  SDL_Surface* surf = IMG_Load("romfs:/failed.png");
  s_refresh = SDL_CreateTextureFromSurface(Screen::renderer, surf);
  SDL_FreeSurface(surf);
}

void Screen::cleanup_texture(SDL_Texture* texture){
  if(texture != NULL){
    if(texture != s_refresh){
      SDL_DestroyTexture(texture);
    }
    texture = NULL;
  }
}

// Clean up SDL, free resources etc
void Screen::close()
{
  // Fonts
  TTF_CloseFont(Screen::gallery_info);
  TTF_CloseFont(Screen::normal);
  TTF_Quit();

  // Textures
  SDL_DestroyTexture(s_refresh);
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

void Screen::draw_button(int x, int y, int w, int h, SDL_Color fore, SDL_Color back, int border){
  // Draw back layer (border)
  if(border > 0)
    draw_rect(x, y, w, h, back);

  // Draw front layer
  draw_rect(x+border, y+border, w-(border*2), h-(border*2), fore);
}

void draw_text_internal(std::string text, int x, int y, SDL_Color color, TTF_Font* font){
  SDL_Surface *surf = TTF_RenderText_Solid(font, text.c_str(), color);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(Screen::renderer, surf);

  SDL_Rect rect = { x, y, surf->w, surf->h };
  SDL_RenderCopy(Screen::renderer, texture, NULL, &rect);
  SDL_FreeSurface(surf);
  SDL_DestroyTexture(texture);
}

void Screen::draw_text_centered(std::string text, int x, int y, int maxw, int maxh, SDL_Color color, TTF_Font* font){
  SDL_Surface *surf = TTF_RenderText_Solid(font, text.c_str(), color);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(Screen::renderer, surf);
  int w, h;
  SDL_QueryTexture(texture, NULL, NULL, &w, &h);

  SDL_Rect rect = { x + (maxw - surf->w)/2, y + (maxh - surf->h)/2,
                   surf->w, surf->h };
  SDL_RenderCopy(Screen::renderer, texture, NULL, &rect);
  SDL_FreeSurface(surf);
  SDL_DestroyTexture(texture);
}

void Screen::draw_text(std::string text, int x, int y, SDL_Color color, TTF_Font* font)
{
  draw_text_internal(text, x, y, color, font);
}

void Screen::draw_text(std::string text, int x, int y, SDL_Color color) {
  draw_text_internal(text, x, y, color, normal);
}
