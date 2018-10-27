#ifndef BROWSER_HPP
#define BROWSER_HPP

#include <json-c/json.h>
#include <vector>
#include <mutex>
#include "Api.hpp"
#include "Shared.hpp"

struct Entry{
  std::string title;
  std::string category;
  int rating;
  std::string thumb;
  std::string url;
  SDL_Texture* thumb_texture;
  int thumb_loaded;
  int pages;

  Resource* res;
  Entry(){
    thumb_texture = NULL;
  }
};

class Browser {
  public:
    static void close();
    static void set_touch();

    static Entry new_entry(json_object* json, int num);
    static void add_entry(Entry entry);
    static void render_entry(Entry* entry, int x, int y, bool active);
    static void render();
    static void clear();
    static Handler on_event(int val);

    static const int maxw = 128; // Thumb width
    static const int maxh = 181; // Thumb height
    static const int maxw2 = 128 + 128 + 5 + 65; // Full width#

    static int active_gallery;
    static int grid_start;
    static std::vector<Entry> entries;
};

#endif
