#pragma once

#include "Browser.hpp"
#include "Shared.hpp"

class GalleryPreview {
  public:
    static void load_gallery(Entry* entry);
    static void set_touch();
    static Handler on_event(int val);
    static void render();
  private:
    static Entry* entry;
};