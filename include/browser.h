#ifndef BROWSER_H
#define BROWSER_H

#include <json-c/json.h>

struct Entry{
  const char* title;
  const char* category;
  int rating;
  const char* thumb;
};

class Browser {
  public:
    static Entry add_entry(json_object* json, int num);
    static void render_entry(Entry entry, int x, int y);

    static const int maxw = 300; // Thumb width
    static const int maxh = 500; // Thumb height
};

#endif
