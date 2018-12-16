#pragma once

#include "RegexHelper.hpp"
#include "Browser.hpp"
#include "Shared.hpp"
#include "Api.hpp"
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>

struct Gallery{
  Entry* entry;
  std::string title;
  std::string index;
  std::vector<Resource*> images;
  int total_pages;

  ~Gallery(){
    for(auto res : images){
      ApiManager::cleanup_resource(res);
    }
  }
};

class GalleryBrowser : public Handler {
  public:
    static void close();
    static void load_gallery(Entry* entry);
    static void set_touch();
    static void gesture(SDL_Event e);
    static void scroll(float dx, float dy);
    static void finger_down(SDL_Event e);
    static void finger_up(SDL_Event e);

    static void load_page(int page);
    static void load_urls(size_t page);
    static int save_all_pages();
    static void handle_req(Resource* res);

    static HandlerEnum on_event(int val);

    static void render();

    static int block_size;
    static int cur_page;
    static Gallery* active_gallery;
    static const int buffer_size;
    static xmlXPathObjectPtr get_node_set(xmlDocPtr doc, xmlChar *xpath);
  private:
    static void set_pos_bounds();
};