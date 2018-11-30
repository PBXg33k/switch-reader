#pragma once

#include "Browser.hpp"
#include "Shared.hpp"

class GalleryPreview : public Handler {
  public:
    static void load_gallery(Entry* entry);
    static void set_touch();
    static HandlerEnum on_event(int val);
    static void scroll(float dx, float dy);
    static void render();
  private:
    static Entry* entry;
};