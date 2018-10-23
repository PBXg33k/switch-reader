#ifndef UI_H
#define UI_H

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

class Screen {
  public:
    static void init();
    static void close();
    static void draw_image(std::string path, int x, int y);
    static void draw_image_mem(char* data, size_t len, int x, int y);
    static void draw_entry_mem(char* data, size_t len, int x, int y);
    static void draw_text(std::string text, int x, int y, SDL_Color color);
    static void render();

    static inline SDL_Renderer *renderer;
    static inline SDL_Window *window;

    static inline TTF_Font *normal = NULL;
};

#endif
