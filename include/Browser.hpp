#pragma once

#include <json-c/json.h>
#include <vector>
#include <mutex>
#include "Api.hpp"
#include "Shared.hpp"

class Browser {
  public:
    static void close();
    static void set_touch();
    static void quit_app();

    static void load_username();
    static void new_entry(json_object* json, Entry* entry, int num, std::string url);
    static void add_entry(Entry* entry);
    static void render_entry(Entry* entry, int x, int y, bool active);
    static void render();
    static void clear();
    static void scroll(float dx);
    static Handler on_event(int val);

    static const int maxw = 128; // Thumb width
    static const int maxh = 181; // Thumb height
    static const int maxw2 = 128 + 128 + 5 + 65; // Full width of a panel

    static int numOfResults;
    static std::string currentUrl;
    static float scroll_pos;
    static int active_gallery;
    static std::vector<Entry*> entries;
    static int loadedPages;
    static std::string username;
  private:
    static void load_urls();
};