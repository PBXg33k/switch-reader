#pragma once

#include <gif_lib.h>

namespace Gif_Renderer {
  GifFileType* get_test_gif();

  void render(GifFileType* gif, int frame);
}