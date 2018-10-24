#ifndef BROWSER_H
#define BROWSER_H

#include <json-c/json.h>
#include <vector>
#include "api.h"

struct Entry{
  const char* title;
  const char* category;
  int rating;
  const char* thumb;
  const char* url;
  MemoryStruct thumb_data;
  int thumb_loaded;
};

class Browser {
  public:
    static void set_touch();

    static Entry new_entry(json_object* json, int num);
    static void add_entry(Entry entry);
    static void render_entry(Entry* entry, int x, int y);
    static void render();

    static const int maxw = 128; // Thumb width
    static const int maxh = 181; // Thumb height
    static const int maxw2 = 128 + 128 + 5 + 65; // Full width

    static int grid_start;
    static std::vector<Entry> entries;
};

#endif
