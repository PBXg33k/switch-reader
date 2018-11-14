#pragma once

#include <string>
#include <switch.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

class Screen {
  public:
    static void init();
    static void close();
    static SDL_Texture* load_texture(char* image, size_t size);
    static SDL_Texture* load_stored_texture(int id);
    static void cleanup_texture(SDL_Texture* texture);
    static void draw_image_mem(SDL_Texture* texture, int x, int y);
    static void draw_partial(int x, int y, double percentX, double percentY, SDL_Texture* texture);
    static void draw_adjusted_mem(SDL_Texture* texture, int x, int y, int maxw, int maxy, int rot = 0);
    static void draw_text(std::string text, int x, int y, SDL_Color color);
    static void draw_text(std::string text, int x, int y, SDL_Color color, TTF_Font* font);
    static void draw_text_centered(std::string text, int x, int y, int maxw, int maxh, SDL_Color color, TTF_Font* font);
    static void draw_rect(int x, int y, int w, int h, SDL_Color color);
    static void draw_pill(int x, int y, int w, int h, SDL_Color color);
    static void draw_button(int x, int y, int w, int h, SDL_Color fore, SDL_Color back, int border_thickness);
    static void clear(SDL_Color color);
    static void render();

    static inline SDL_Renderer *renderer;
    static inline SDL_Window *window;
    static inline SDL_Surface *surface;

    static inline PlFontData standardFontData;
    static inline TTF_Font *gallery_info;
    static inline TTF_Font *normal;
    static inline TTF_Font *large;

    static SDL_Texture* s_stars;
};