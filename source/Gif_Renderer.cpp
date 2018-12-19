#include "Gif_Renderer.hpp"
#include "Api.hpp"

#define testGifPath "/test.gif"

GIF_Image* Gif_Renderer::get_test_gif(){
  printf("Loading test gif\n");
  return GIF_LoadImage(testGifPath);
}

void Gif_Renderer::render(GIF_Image* gif, int frame){
  printf("Rendering gif\n");
  if(gif != NULL){
    // Render
  }
}