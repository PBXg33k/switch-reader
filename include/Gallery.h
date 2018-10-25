#ifndef GALLERY_H
#define GALLERY_H

#include "RegexHelper.h"
#include "Browser.h"
#include "shared.h"

typedef struct Gallery{
  std::string title;
  std::string index;
  std::vector<const char*> pages;
  int cur_page;
}Gallery;

class GalleryBrowser {
  public:
    static void load_gallery(Entry* entry);
    static void set_touch();

    static void load_page(int page);
    static void load_urls(int page);

    static Handler on_event(int val);

    static void render();

    static Gallery* active_gallery;
    static SDL_Texture* active_image;
};

#endif
