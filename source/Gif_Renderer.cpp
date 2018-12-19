#include "Gif_Renderer.hpp"
#include "Api.hpp"

#define testGifPath "/test.gif"

GifFileType* Gif_Renderer::get_test_gif(){
  printf("Loading gif\n");
  int error = 0;
  GifFileType* gif = DGifOpenFileName(testGifPath, &error);

  printf("Slurping - %d\n", error);
  if(!error)
    DGifSlurp(gif);
  return gif;
}

void Gif_Renderer::render(GifFileType* gif, int frame){
  printf("Rendering gif\n");
  if(gif != NULL){
    // Render
  }
}