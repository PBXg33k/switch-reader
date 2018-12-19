
#include "Api.hpp"
#include "Config.hpp"

#include <fstream>

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

void GIF_Info::load_gif(MemoryStruct* mem){
  // Save to temp file
  temp_path = ConfigManager::downloadsDir + "/" + std::to_string(SDL_GetTicks()) + ".gif";
  printf("Making temp file at %s\n", temp_path.c_str());
  FILE* file = fopen(temp_path.c_str(), "wb");
  fwrite(mem->memory, 1, mem->size, file);
  fclose(file);

  // Load as GIF
  gif = GIF_LoadImage(temp_path.c_str());
}