#ifndef BROWSER_H
#define BROWSER_H

#include <json-c/json.h>
#include "api.h"

struct Entry{
  const char* title;
  const char* category;
  int rating;
  const char* thumb;
  MemoryStruct thumb_data;
  int thumb_loaded;
};

class Browser {
  public:
    static Entry add_entry(json_object* json, int num);
    static void render_entry(Entry* entry, int x, int y);

    static const int maxw = 128; // Thumb width
    static const int maxh = 181; // Thumb height
    static const int maxw2 = 128 + 128 + 5 + 65; // Full width 
};

#endif
