#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#include "Preview.hpp"
#include "Shared.hpp"
#include "Gallery.hpp"
#include "Touch.hpp"
#include "Ui.hpp"

#define tags_x 440
#define tags_y 86

Entry* GalleryPreview::entry;
static int curX, curY;
static int scroll_pos;
static std::string message;
std::string last_category;

void GalleryPreview::load_gallery(Entry* entry){
    message.clear();
    GalleryPreview::entry = entry;
    scroll_pos = 0;
}

void GalleryPreview::set_touch(){
  TouchManager::clear();

  // Back to browser
  TouchManager::add_bounds(screen_width - 75, 0, 75, 75, 101);

  // Load Gallery
  TouchManager::add_bounds(screen_width-190, (screen_height/2) - 40, 180, 80, 102);

  // Download/Delete Gallery
  TouchManager::add_bounds(screen_width-190, (screen_height/2) + 110, 180, 80, 111);
}

HandlerEnum GalleryPreview::on_event(int val){
  // Load Gallery
  if(val == 102){
    ApiManager::cancel_all_requests();
    GalleryBrowser::load_gallery(entry);
    GalleryBrowser::set_touch();
    return HandlerEnum::Gallery;
  }
  // Download Gallery
  if(val == 111){
    printf("Downloading Gallery\n");
    int ret = ApiManager::download_gallery(entry, nullptr);

    if(ret == 1)
      message = "Download Not Supported";
    else if(ret != 0)
      message = "Download Failed";
    else
      message = "Download Successful";
  }

  // Exit to Browser
  if(val == 101){
    return HandlerEnum::Browser;
  }

  return HandlerEnum::Preview;
}

void render_tag(std::string type, std::string name){
  int w, h;
  w = FC_GetWidth(Screen::normal, name.c_str());
  h = FC_GetHeight(Screen::normal, name.c_str());
  // Next line
  if(curX + w + 10 > screen_width - 200){
    curX = tags_x;
    curY += (h + 21);
  }
  // Render tag here
  Screen::draw_rect(curX, curY, w + 10, h + 6, ThemePanelDark);
  Screen::draw_text(name.c_str(), curX + 5, curY + 3, ThemeText);
  // Next tag
  curX += w + 20;
}

void GalleryPreview::render(){
  Screen::clear(ThemeBG);
  last_category = "none";

  // Info message
  if(!message.empty())
    Screen::draw_text_centered(message, 30, screen_height - 100, 240, 80, ThemeText, Screen::large);

  // Load Gallery button
  Screen::draw_button(screen_width-190, (screen_height/2) - 40, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Load Gallery", screen_width-190, (screen_height/2) - 40, 180, 80, ThemeButtonText, Screen::normal);

  // Download/Delete Gallery button
  Screen::draw_button(screen_width-190, (screen_height/2) + 110, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  if(entry->local)
    Screen::draw_text_centered("Delete", screen_width-190, (screen_height/2) + 110, 180, 80, ThemeButtonText, Screen::normal);
  else
    Screen::draw_text_centered("Download", screen_width-190, (screen_height/2) + 110, 180, 80, ThemeButtonText, Screen::normal);

  // Load thumbnail
  if(!entry->res->requested){
    ApiManager::request_res(entry->res);
  }

  // Draw thumbnail
  if(entry->res->texture){
    Screen::draw_adjusted_mem(entry->res->texture, 30, 30, 240, 450);
  }

  // Draw tags
  curX = tags_x;
  curY = tags_y - scroll_pos;
  for(auto tag : entry->tags){
    if(tag.first != last_category){
      curX = tags_x;
      curY += 42;
      Screen::draw_text(tag.first, tags_x - 140, curY, ThemeText);
      last_category = tag.first;
    }
    render_tag(tag.first, tag.second);
  }

    // Draw title
  Screen::draw_rect(tags_x - 140, 0, screen_width - (tags_x - 140), tags_y, ThemeBG);
  Screen::draw_text(entry->title, tags_x - 140, tags_y - 40, ThemeText, Screen::large);

  // Back button
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);
}

void GalleryPreview::scroll(float dx, float dy) {
  float amount = screen_height * dy;

  if(scroll_pos - amount >= 0){
    scroll_pos = scroll_pos - amount;
  } else {
    scroll_pos = 0;
  }
};