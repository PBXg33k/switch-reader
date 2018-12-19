#pragma once

#include "SDL_Gifwrap.h"

namespace Gif_Renderer {
  GIF_Image* get_test_gif();

  void render(GIF_Image* gif, int frame);
}