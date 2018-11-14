#include "Ui.hpp"
#include "Api.hpp"
#include "Browser.hpp"
#include "Touch.hpp"
#include "HSearch.hpp"
#include "Gallery.hpp"
#include "Preview.hpp"
#include "Search.hpp"
#include "Settings.hpp"
#include <iostream>
#include <math.h>

int Browser::active_gallery = -1;
std::string Browser::currentUrl;
int Browser::numOfResults = 0;
float Browser::scroll_pos = 0;
std::vector<Entry*> Browser::entries = std::vector<Entry*>();

void Browser::close(){
  for(auto e : entries){
    if(e->res->mem){
      delete e->res->mem;
    }
  }
}

json_object* get_json_obj(json_object* root, const char* key)
{
  json_object* ret;
  if (json_object_object_get_ex(root, key, &ret)){
    return ret;
  }
  return NULL;
}

// Set up bounding boxes with paired values
void Browser::set_touch(){
  TouchManager::clear();

  // Settings
  TouchManager::add_bounds(screen_width-190, (screen_height/2) + 110, 180, 80, 111);
  // Search
  TouchManager::add_bounds(screen_width-190, (screen_height/2) - 190, 180, 80, 110);
  // Load Gallery
  TouchManager::add_bounds(screen_width-190, (screen_height/2) - 40, 180, 80, 102);
  // Quit app
  TouchManager::add_bounds(screen_width - 75, 0, 75, 75, 101);
  // Stop pressing in button backgrounds
  TouchManager::add_bounds(screen_width-200, 0, 200, screen_height, 1000);

  int baseX = 30;
  int baseY = 30;
  int incX = (Browser::maxw2 + 30);
  int incY = (Browser::maxh + 30);
  int val = 0;
  int offset = ((int) scroll_pos) % incX;

  for (int x = 0; x < 4; x++){
    for (int y = 0; y < 3; y++){
      int newX = baseX + (x * incX) - offset;
      int newY = baseY + (y * incY);
      TouchManager::add_bounds(newX, newY, Browser::maxw2, Browser::maxh, val);
      val++;
    }
  }
}

Entry* Browser::new_entry(json_object* json, int num, std::string url)
{
  json_object *holder;
  Entry* entry = new Entry;

  // Populate entry
  json = json_object_array_get_idx(json, num);

  holder = get_json_obj(json, "gid");
  entry->id = json_object_get_int(holder);

  holder = get_json_obj(json, "title");
  entry->title = json_object_get_string(holder);

  holder = get_json_obj(json, "category");
  entry->category = json_object_get_string(holder);

  holder = get_json_obj(json, "rating");
  entry->rating = json_object_get_double(holder);

  holder = get_json_obj(json, "thumb");
  entry->thumb = json_object_get_string(holder);

  holder = get_json_obj(json, "filecount");
  entry->pages = json_object_get_int(holder);

  entry->url = url;

  add_entry(entry);
  return entry;
}

// Add entry to list of entries
void Browser::add_entry(Entry* entry){
  // Select first gallery by default
  if(entries.empty())
    active_gallery = 0;

  Browser::entries.push_back(entry);
}

void Browser::clear(){
  scroll_pos = 0;
  for(auto entry : entries){
    delete entry;
  }
  Browser::entries.clear();
}

void Browser::render(){
  Screen::clear(ThemeBG);

  int baseX = 30;
  int baseY = 30;
  int incX = (Browser::maxw2 + 30);
  int incY = (Browser::maxh + 30);
  // Index to start from
  int idx = (scroll_pos / incX) * 3;
  idx -= (idx % 3);

  int offset = ((int) scroll_pos) % incX;

  // Load more if not enough

  if(idx + 11 >= (int) entries.size()){
    load_urls();
  }

  // Render upto 4X3 grid, based on start point.
  for (int x = 0; x < 4; x++){
    for(int y = 0; (y < 3) && (idx < (int) entries.size()); y++){
      bool active_gal = (idx == Browser::active_gallery);
      Entry* entry = Browser::entries[idx];
      Browser::render_entry(entry, baseX + (x * incX) - offset, baseY + (y * incY), active_gal);
      idx++;
    }
  }

  // Clean background for buttons
  Screen::draw_rect(screen_width - 200, 0, 200, screen_height, ThemeBG);
  // Settings button
  Screen::draw_button(screen_width-190, (screen_height/2) + 110, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Settings", screen_width-190, (screen_height/2) + 110, 180, 80, ThemeButtonText, Screen::normal);
  // Search button
  Screen::draw_button(screen_width-190, (screen_height/2) - 190, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Search", screen_width-190, (screen_height/2) - 190, 180, 80, ThemeButtonText, Screen::normal);
  // Load Gallery button
  Screen::draw_button(screen_width-190, (screen_height/2) - 40, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Load Gallery", screen_width-190, (screen_height/2) - 40, 180, 80, ThemeButtonText, Screen::normal);
  // Quit button
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);
}

void Browser::render_entry(Entry* entry, int x, int y, bool active)
{
  // If image not loaded, stick in texture
  if(entry->res->requested == 0){
    //printf("Requesting thumb texture\n");
    //entry->res = new Resource();
    //mutexInit(mutex);
    entry->res->url = entry->thumb;
    ApiManager::request_res(entry->res);
    entry->res->requested = 1;
  }

  std::string new_title = entry->title;
  new_title.resize(25);

  SDL_Color imgFG = ThemePanelLight;
  SDL_Color imgBG = ThemePanelDark;
  if(active){
    imgFG = ThemePanelSelectedLight;
    imgBG = ThemePanelSelectedDark;
  }

  Screen::draw_rect(x-5, y-5, maxw+10, maxh+10, imgFG);
  Screen::draw_rect(x + maxw+5, y-5, maxw+65, maxh+10, imgBG);

  if(entry->res->texture){
    Screen::draw_adjusted_mem(entry->res->texture, x, y, maxw, maxh);
  }

  // Title -> Category -> Pages -> -> Rating -> Language
  Screen::draw_text(new_title, x + maxw + 10, y + 5, ThemeText, Screen::gallery_info);

  Screen::draw_text(entry->category, x + maxw + 10, y + 35, ThemeText, Screen::normal);
  
  Screen::draw_text((std::to_string(entry->pages) + " Pages").c_str(), x + maxw + 10, y + 65, ThemeText, Screen::normal);

  Screen::draw_partial(x + maxw + 10, y + 108, entry->rating / (double) 5, 1, Screen::s_stars);

  if(!entry->language.empty())
    Screen::draw_text(entry->language.c_str(), x + maxw + 10, y + (maxh - 24), ThemeText, Screen::normal);
}

Handler Browser::on_event(int val){
  int offset;
  // Select Gallery
  if(val >= 0 && val < 13){
    offset = (scroll_pos / (maxw2 + 30)) * 3;
    offset -= (offset % 3);
    printf("Real %d - Adjusted %d\n", val, val + offset);
    Browser::active_gallery = val + offset;
  // Change to Gallery
  } else if(Browser::active_gallery >= 0 && val == 102){
    Entry* entry = Browser::entries[active_gallery];
    printf("URL %s\n", entry->url.c_str());
    ApiManager::cancel_all_requests();
    GalleryPreview::load_gallery(entry);
    GalleryPreview::set_touch();
    return Handler::Preview;
    // GalleryBrowser::load_gallery(entry);
    // GalleryBrowser::set_touch();
    // return Handler::Gallery;
  // Change to Search
  } else if (val == 110) {
    active_gallery = -1;
    SearchBrowser::set_touch();
    return Handler::Search;
  // Gallery selection
  } else if (val >= 120 && val < 130 && active_gallery >= 0){
    // O - Up, clockwise rot
    int dir = val - 120;

    switch(dir){
      case 0:
        if(active_gallery % 3 > 0)
          active_gallery--;
      case 1:
        if(active_gallery < numOfResults - 2)
          active_gallery += 3;
        break;
      case 2:
        if(active_gallery % 3 != 2)
          active_gallery++;
        break;
      case 3:
        if(active_gallery > 2)
          active_gallery -= 3;
        break;
      default:
        break;
    }
  } else if (val == 111){
    Settings::set_touch();
    return Handler::Settings;
  } else if (val == 101){
    quit_app();
  }

  return Handler::Browser;
}

// Scrolls based on a normalized float - Screen moves left as number rises
void Browser::scroll(float dx){
  float amount = screen_width * dx;
  int incX = (Browser::maxw2 + 30);
  int new_pos;
  Entry* entry;
  int i;

  if(scroll_pos - amount >= 0){
    new_pos = scroll_pos - amount;
  } else {
    new_pos = 0;
  }

  int cur_idx = (scroll_pos / incX) * 3;
  cur_idx -= (cur_idx % 3);
  int new_idx = (new_pos / incX) * 3;
  new_idx -= (new_idx % 3);

  // Scrolled backwards
  if(new_idx < cur_idx){
    for(i = new_idx + 12; i < (int)entries.size() && i < new_idx + 16; i++){
      entry = entries[i];
      entry->res->requested = 0;
      if(entry->res->texture){
        SDL_DestroyTexture(entry->res->texture);
        entry->res->texture = NULL;
      }
    }
  }

  // Scrolled forwards
  else if(new_idx > cur_idx){
    for(i = new_idx - 3; i < (int)entries.size() && i < new_idx; i++){
      entry = entries[i];
      entry->res->requested = 0;
      if(entry->res->texture){
        SDL_DestroyTexture(entry->res->texture);
        entry->res->texture = NULL;
      }
    }
  }

  scroll_pos = new_pos;

  set_touch();
}

void Browser::load_urls(){
  if(entries.size() < (size_t) numOfResults){
    int page = entries.size() / 25;
    HSearch::expand_search(currentUrl, page);
  }
}