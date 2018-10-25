#ifndef GALLERY_H
#define GALLERY_H

#include "RegexHelper.h"
#include "Browser.h"
#include "shared.h"
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>

typedef struct Gallery{
  std::string title;
  std::string index;
  std::vector<std::string> pages;
}Gallery;

class GalleryBrowser {
  public:
    static void load_gallery(Entry* entry);
    static void set_touch();

    static void load_page(int page);
    static void load_urls(int page);

    static Handler on_event(int val);

    static void render();

    static int cur_page;
    static Gallery* active_gallery;
    static SDL_Texture* active_image;
    static bool clear_next_render;
  private:
    static xmlXPathObjectPtr get_node_set(xmlDocPtr doc, xmlChar *xpath);
};

#endif
