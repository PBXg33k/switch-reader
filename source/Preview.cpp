#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#include "Preview.hpp"
#include "Shared.hpp"
#include "Gallery.hpp"
#include "Touch.hpp"
#include "Ui.hpp"

#define tags_x 400
#define tags_y 80

Entry* GalleryPreview::entry;
static int curX, curY;
std::string last_category;

void GalleryPreview::load_gallery(Entry* entry){
    GalleryPreview::entry = entry;
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

Handler GalleryPreview::on_event(int val){
  // Load Gallery
  if(val == 102){
    printf("URL %s\n", entry->url.c_str());
    ApiManager::cancel_all_requests();
    GalleryBrowser::load_gallery(entry);
    GalleryBrowser::set_touch();
    return Handler::Gallery;
  }

  // Download Gallery
  if(val == 111){
    printf("Downloading Gallery\n");
    ApiManager::download_gallery(entry, nullptr);
  }

  // Exit to Browser
  if(val == 101){
    return Handler::Browser;
  }

  return Handler::Preview;
}

void render_tag(Tag tag){
  int w, h;
  // Can render tag
  if(!TTF_SizeText(Screen::normal, tag.tag.c_str(), &w, &h)){
    // Next line
    if(curX + w + 10 > screen_width - 200){
      curX = tags_x;
      curY += (h + 21);
    }
    // Render tag here
    Screen::draw_rect(curX, curY, w + 10, h + 6, ThemePanelDark);
    Screen::draw_text(tag.tag.c_str(), curX + 5, curY + 3, ThemeText);
    // Next tag
    curX += w + 20;
  }
}

void GalleryPreview::render(){
  Screen::clear(ThemeBG);
  last_category = "none";

  // Load Gallery button
  Screen::draw_button(screen_width-190, (screen_height/2) - 40, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Load Gallery", screen_width-190, (screen_height/2) - 40, 180, 80, ThemeButtonText, Screen::normal);

  // Download/Delete Gallery button
  Screen::draw_button(screen_width-190, (screen_height/2) + 110, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  if(entry->local)
    Screen::draw_text_centered("Delete", screen_width-190, (screen_height/2) + 110, 180, 80, ThemeButtonText, Screen::normal);
  else
    Screen::draw_text_centered("Download", screen_width-190, (screen_height/2) + 110, 180, 80, ThemeButtonText, Screen::normal);

  // Back button
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);

  // Load thumbnail
  if(!entry->res->requested){
    ApiManager::request_res(entry->res);
  }

  // Draw thumbnail
  if(entry->res->texture){
    Screen::draw_adjusted_mem(entry->res->texture, 30, 30, 240, 450);
  }

  // Draw title
  Screen::draw_text(entry->title, tags_x - 100, tags_y - 10, ThemeText, Screen::large);

  // Draw tags
  curX = tags_x;
  curY = tags_y;
  for(Tag tag : entry->tags){
    if(tag.category != last_category){
      curX = tags_x;
      curY += 42;
      Screen::draw_text(tag.category, tags_x - 100, curY, ThemeText);
      last_category = tag.category;
    }
    render_tag(tag);
  }
}