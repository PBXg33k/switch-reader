#include "Ui.hpp"
#include "Api.hpp"
#include "Browser.hpp"
#include "Touch.hpp"
#include "HSearch.hpp"
#include "Gallery.hpp"
#include "Preview.hpp"
#include "Search.hpp"
#include "Config.hpp"
#include "Settings.hpp"
#include <iostream>
#include <math.h>

#define usernameXPath "//a[contains(@href, '?showuser=')]"

int Browser::active_gallery = -1;
std::string Browser::currentUrl;
int Browser::numOfResults = 0;
int Browser::loadedPages = 0;
float Browser::scroll_pos = 0;
std::string Browser::username;
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

void Browser::load_username(){
  xmlChar *path;
  xmlChar *keyword = NULL;
  xmlXPathObjectPtr result;
  xmlDocPtr doc;
  xmlNodeSetPtr nodeset;

  username = "User Check Failed";

  MemoryStruct* pageMem = new MemoryStruct();
  ApiManager::get_res(pageMem, UserURL.c_str());

  // If page failed to load, return failure image
  if(pageMem->size == 0){
    delete pageMem;
    return;
  }
  printf("Loaded user page\n");

  doc = htmlReadMemory(pageMem->memory, pageMem->size, UserURL.c_str(), NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

  if(doc == nullptr){
    printf("Page empty\n");
    xmlCleanupParser();
    delete pageMem;
    return;
  }

  // Get node list matching XPath for direct image url
  path = (xmlChar*) usernameXPath;
  result = GalleryBrowser::get_node_set(doc, path);
  
  if(!xmlXPathNodeSetIsEmpty(result->nodesetval)){
    printf("User found\n");
    nodeset = result->nodesetval;
    keyword = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode, 1);
    printf("Casting\n");
    username = std::string(reinterpret_cast<char*> (keyword));
  } else {
    printf("No user found logged in\n");
    username = "Not Logged In";
  }

  printf("Cleaning up\n");

  xmlFree(keyword);
  xmlFreeDoc(doc);
  xmlCleanupParser();
  delete pageMem;

}

// Set up bounding boxes with paired values
void Browser::set_touch(){
  TouchManager::instance.clear();

  // Settings
  TouchManager::instance.add_bounds(screen_width-190, (screen_height/2) + 140, 180, 80, 111);
  // Search
  TouchManager::instance.add_bounds(screen_width-190, (screen_height/2) - 190, 180, 80, 110);
  // Load Gallery
  TouchManager::instance.add_bounds(screen_width-190, (screen_height/2) - 40, 180, 80, 102);

  // E-Hentai specific button
  if(ConfigManager::get_value("domain") != "NHentai"){
    // Favourites
    TouchManager::instance.add_bounds(screen_width-190, (screen_height/2) + 50, 180, 80, 115);
  }


  // Quit app
  TouchManager::instance.add_bounds(screen_width - 75, 0, 75, 75, 100);
  // Stop pressing in button backgrounds
  TouchManager::instance.add_bounds(screen_width-200, 0, 200, screen_height, 1000);

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
      TouchManager::instance.add_bounds(newX, newY, Browser::maxw2, Browser::maxh, val);
      val++;
    }
  }
}

void Browser::new_entry(json_object* json, Entry* entry, int num, std::string url)
{
  json_object *holder;

  // Populate entry
  json = json_object_array_get_idx(json, num);

  holder = get_json_obj(json, "gid");
  entry->id = json_object_get_int(holder);

  holder = get_json_obj(json, "title");
  entry->title = json_object_get_string(holder);

  printf("Adding Entry %s\n", entry->title.c_str());

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
  if(idx + 11 >= (int) entries.size() && (int) entries.size()+1 < numOfResults){
    printf("Loading from %d of %d\n", (int) entries.size()+1, numOfResults);
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

  // No results
  if(entries.size() == 0){
    Screen::draw_text_centered("No Results :(", 0, 0, screen_width - 200, screen_height - 40, ThemeText, Screen::header);
  }

  // Clean background for buttons
  Screen::draw_rect(screen_width - 200, 0, 200, screen_height, ThemeBG);
  // Settings button
  Screen::draw_button(screen_width-190, (screen_height/2) + 140, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Settings", screen_width-190, (screen_height/2) + 140, 180, 80, ThemeButtonText, Screen::normal);
  // Search button
  Screen::draw_button(screen_width-190, (screen_height/2) - 190, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Search", screen_width-190, (screen_height/2) - 190, 180, 80, ThemeButtonText, Screen::normal);
  // Load Gallery button
  Screen::draw_button(screen_width-190, (screen_height/2) - 40, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Load Gallery", screen_width-190, (screen_height/2) - 40, 180, 80, ThemeButtonText, Screen::normal);

  // E-Hentai specific
  if(ConfigManager::get_value("domain") != "NHentai"){
    // Favourites button
    Screen::draw_button(screen_width-190, (screen_height/2) + 50, 180, 80, ThemeButton, ThemeButtonBorder, 4);
    Screen::draw_text_centered("Favourites", screen_width-190, (screen_height/2) + 50, 180, 80, ThemeButtonText, Screen::normal);

    // Username
    Screen::draw_text(username.c_str(), 30, screen_height - 40, ThemeText, Screen::large);
  }

  // Mode
  Screen::draw_text_aligned(ConfigManager::get_value("domain"), 0, screen_height-40, screen_width - 210, 40, FC_ALIGN_RIGHT, ThemeText, Screen::large);

  // Quit button
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);
}

void Browser::render_entry(Entry* entry, int x, int y, bool active)
{
  // If image not loaded, stick in texture
  if(entry->res->requested == 0){
    //printf("Requesting thumb texture\n");
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

HandlerEnum Browser::on_event(int val){
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
    GalleryPreview::load_gallery(entry);
    GalleryPreview::set_touch();
    return HandlerEnum::Preview;
  // Change to Search
  } else if (val == 110) {
    active_gallery = -1;
    scroll_pos = 0;
    ApiManager::cancel_all_requests();
    SearchBrowser::set_touch();
    return HandlerEnum::Search;
  // Load Favourites
  } else if (val == 115){
    active_gallery = -1;
    scroll_pos = 0;
    Browser::clear();
    Domain* domain = HSearch::current_domain();
    domain->search_favourites();
    return HandlerEnum::Browser;
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
  // Settings
  } else if (val == 111){
    Settings::set_touch();
    return HandlerEnum::Settings;
  }

  return HandlerEnum::Browser;
}

// Scrolls based on a normalized float - Screen moves left as number rises
void Browser::scroll(float dx, float dy){
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
        Screen::cleanup_texture(entry->res->texture);
        entry->res->texture = Screen::s_loading;
      }
    }
  }

  // Scrolled forwards
  else if(new_idx > cur_idx){
    for(i = new_idx - 3; i < (int)entries.size() && i < new_idx; i++){
      entry = entries[i];
      entry->res->requested = 0;
      if(entry->res->texture){
        Screen::cleanup_texture(entry->res->texture);
        entry->res->texture = Screen::s_loading;
      }
    }
  }

  scroll_pos = new_pos;

  set_touch();
}

void Browser::load_urls(){
  if(entries.size() < (size_t) numOfResults){
    loadedPages++;
    HSearch::expand_search(currentUrl, loadedPages);
  }
}