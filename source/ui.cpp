#include "ui.h"

void Screen::init()
{
  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_JPG);
  TTF_Init();

  plInitialize();

  Result rc;
  rc = plGetSharedFontByType(&Screen::standardFontData, PlSharedFontType_Standard);
  if(!R_FAILED(rc)){
    Screen::gallery_info = TTF_OpenFontRW(SDL_RWFromMem(Screen::standardFontData.address, Screen::standardFontData.size), 1, 18);
    Screen::normal = TTF_OpenFontRW(SDL_RWFromMem(Screen::standardFontData.address, Screen::standardFontData.size), 1, 24);
  }

  // Set up windows
  Screen::window = SDL_CreateWindow("Image Test",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, 0);
  Screen::renderer = SDL_CreateRenderer(window, -1, 0);
}

// Clean up SDL, free resources etc
void Screen::close()
{
  TTF_CloseFont(Screen::normal);
  TTF_Quit();
  IMG_Quit();
  SDL_DestroyRenderer(Screen::renderer);
  SDL_DestroyWindow(Screen::window);
  SDL_Quit();
}

void Screen::render(){
  SDL_RenderPresent(Screen::renderer);
}

void fit_image(SDL_Surface *image, int x, int y)
{
  if(image){
    SDL_Texture *texture = SDL_CreateTextureFromSurface(Screen::renderer, image);

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
  SDL_FreeSurface(image);
}

// Draw image from memory
void Screen::draw_image_mem(char* data, size_t size, int x, int y){
  SDL_RWops *rw = SDL_RWFromMem(data, size);
  SDL_Surface *image = IMG_Load_RW(rw, 1);
  fit_image(image, x, y);
}

void Screen::draw_adjusted_mem(char*data, size_t len, int x, int y, int maxw, int maxh){
  SDL_RWops *rw = SDL_RWFromMem(data, len);
  SDL_Surface *image = IMG_Load_RW(rw, 1);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(Screen::renderer, image);

  int w, h;
  SDL_QueryTexture(texture, NULL, NULL, &w, &h);
  float scaler = maxh / ((float)(h));
  SDL_Rect rect;
  rect.w = (int)(w * scaler);
  rect.h = (int)(h * scaler);
  rect.x = x;
  rect.y = y;

  scaler = maxw / ((float)(rect.w));
  if(scaler < 1){
    rect.w *= scaler;
    rect.h *= scaler;
  }
  SDL_RenderCopy(Screen::renderer, texture, NULL, &rect);
  SDL_FreeSurface(image);
}

// Draw image to screen at co-ords
void Screen::draw_image(std::string path, int x, int y){
  SDL_Surface *image = IMG_Load(path.c_str());
  fit_image(image, x, y);
}

void draw_text_internal(std::string text, int x, int y, SDL_Color color, TTF_Font* font){
  SDL_Surface *surf = TTF_RenderText_Solid(Screen::normal, text.c_str(), color);

  SDL_Texture *texture = SDL_CreateTextureFromSurface(Screen::renderer, surf);

  SDL_Rect position = { x, y, surf->w, surf->h };
  SDL_RenderCopy(Screen::renderer, texture, NULL, &position);
  SDL_FreeSurface(surf);
}

void Screen::draw_text(std::string text, int x, int y, SDL_Color color, TTF_Font* font)
{
  draw_text_internal(text, x, y, color, font);
}

void Screen::draw_text(std::string text, int x, int y, SDL_Color color) {
  draw_text_internal(text, x, y, color, normal);
}
