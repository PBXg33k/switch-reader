#pragma once

#include "SDL_Gifwrap.h"

struct GIF_Info{
  GIF_Image* gif;
  int frame;
  SDL_Texture* texture;
  unsigned long next_render;
  unsigned long delay;

  SDL_Texture* get_texture();

  GIF_Info(){
    next_render = 0L;
    frame = 0;
    delay = 0L;
    gif = nullptr;
    texture = nullptr;
  }

  ~GIF_Info(){
    if(texture != nullptr)
      SDL_DestroyTexture(texture);
    if(gif != nullptr)
      GIF_FreeImage(gif);
  }
};

GIF_Info* get_test_gif();