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
    static void draw_image_mem(char* data, size_t size, int x, int y);
    static void draw_adjusted_mem(char* data, size_t size, int x, int y, int maxw, int maxy);
    static void draw_text(std::string text, int x, int y, SDL_Color color);
    static void draw_text(std::string text, int x, int y, SDL_Color color, TTF_Font* font);
    static void render();

    static inline SDL_Renderer *renderer;
    static inline SDL_Window *window;

    static inline TTF_Font *gallery_info;
    static inline TTF_Font *normal;
    static inline TTF_Font *large;
};

#endif
