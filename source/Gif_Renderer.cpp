#include "Gif_Renderer.hpp"
#include "Api.hpp"

#define testGifPath "/test.gif"

GIF_Info* get_test_gif(){
  printf("Loading test gif\n");
  GIF_Info* gif = new GIF_Info();
  gif->gif = GIF_LoadImage(testGifPath);
  return gif;
}

SDL_Texture* GIF_Info::get_texture(){
  // Check if ready to change
  if(SDL_GetTicks() > next_render){
    // Cleanup last frame
    if(texture != nullptr)
      SDL_DestroyTexture(texture);

    // Stash new frame
    texture = SDL_CreateTextureFromSurface(Screen::renderer, gif->frames[frame]->surface);

    // Update delays
    frame++;
    if(frame >= gif->num_frames)
      frame = 0;
    delay = gif->frames[frame]->delay;
    // Keep it at the right speed, play catchup if needed
    next_render += delay;
    while(next_render < SDL_GetTicks()){
      next_render += delay;
      frame++;
      if(frame >= gif->num_frames)
        frame = 0;
    }
  }

  return texture;
}