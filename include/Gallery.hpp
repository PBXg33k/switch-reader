#ifndef GALLERY_HPP
#define GALLERY_HPP

#include "RegexHelper.hpp"
#include "Browser.hpp"
#include "Shared.hpp"
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>

typedef struct Gallery{
  std::string title;
  std::string index;
  std::vector<std::string> pages;
  int total_pages;
}Gallery;

class GalleryBrowser {
  public:
    static void close();
    static void load_gallery(Entry* entry);
    static void set_touch();

    static SDL_Texture* load_page(size_t page);
    static void load_urls(size_t page);

    static Handler on_event(int val);

    static void render();

    static int cur_page;
    static Gallery* active_gallery;
    static SDL_Texture* active_image;
    static SDL_Texture* prev_image;
    static SDL_Texture* next_image;
  private:
    static xmlXPathObjectPtr get_node_set(xmlDocPtr doc, xmlChar *xpath);
};

#endif
