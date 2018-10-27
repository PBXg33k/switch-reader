#include "Ui.hpp"
#include "Api.hpp"
#include "Browser.hpp"
#include "TouchManager.hpp"
#include "Gallery.hpp"
#include "Search.hpp"

int Browser::grid_start = 0;
int Browser::active_gallery = -1;
std::vector<Entry> Browser::entries = std::vector<Entry>();

void Browser::close(){
  for(auto e : entries){
    if(e.res->mem){
      delete e.res->mem;
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
  int baseX = 30;
  int baseY = 30;
  int incX = (Browser::maxw2 + 30);
  int incY = (Browser::maxh + 30);
  int val = 0;

  for (int x = 0; x < 3; x++){
    for (int y = 0; y < 3; y++){
      int newX = baseX + (x * incX);
      int newY = baseY + (y * incY);
      TouchManager::add_bounds(newX, newY, Browser::maxw2, Browser::maxh, val);
      val++;
    }
  }

  // Search
  TouchManager::add_bounds(screen_width-190, (screen_height/2) - 190, 180, 80, 11);
  // Load Gallery
  TouchManager::add_bounds(screen_width-190, (screen_height/2) - 40, 180, 80, 10);
  // Quit
  TouchManager::add_bounds(screen_width - 75, 0, 75, 75, 100);
}

Entry* Browser::new_entry(json_object* json, int num, std::string url)
{
  json_object *holder;
  Entry* entry = new Entry;

  // Populate entry
  json = get_json_obj(json, "gmetadata");
  json = json_object_array_get_idx(json, num);

  holder = get_json_obj(json, "title");
  entry->title = json_object_get_string(holder);

  holder = get_json_obj(json, "category");
  entry->category = json_object_get_string(holder);

  holder = get_json_obj(json, "thumb");
  entry->thumb = json_object_get_string(holder);

  holder = get_json_obj(json, "filecount");
  entry->pages = json_object_get_int(holder);

  entry->url = url;

  entry->thumb_loaded = 0;
  return entry;
}

// Add entry to list of entries
void Browser::add_entry(Entry entry){
  //entry.mutex = new Mutex();
  //mutexInit(entry.mutex);
  Browser::entries.push_back(entry);
}

void Browser::clear(){
  Browser::entries.clear();
}

void Browser::render(){
  Screen::clear(ThemeBG);

  int baseX = 30;
  int baseY = 30;
  int incX = (Browser::maxw2 + 30);
  int incY = (Browser::maxh + 30);
  int grid = Browser::grid_start;
  int num_entries = Browser::entries.size() - Browser::grid_start;


  // Render upto 3X3 grid, based on start point.
  for (int x = 0; x < 3; x++){
    for(int y = 0; (y < 3) && (grid < num_entries); y++){
      bool active = ((grid - Browser::grid_start) == Browser::active_gallery);
      Entry* entry = &(Browser::entries[grid]);
      //printf("Render - %s - %s - %s - %s\n", entry->category.c_str(), entry->title.c_str(), entry->url.c_str(), entry->thumb.c_str());
      Browser::render_entry(entry, baseX + (x * incX), baseY + (y * incY), active);
      grid++;
    }
  }

  // Search button
  Screen::draw_button(screen_width-190, (screen_height/2) - 190, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Search", screen_width-190, (screen_height/2) - 190, 180, 80, ThemeButtonText, Screen::normal);
  // Load Gallery button
  Screen::draw_button(screen_width-190, (screen_height/2) - 40, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Load Gallery", screen_width-190, (screen_height/2) - 40, 180, 80, ThemeButtonText, Screen::normal);
  // Quit button
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);
}

Handler Browser::on_event(int val){
  if(val >= 0 && val < 10){
    Browser::active_gallery = val;
  } else if(Browser::active_gallery >= 0 && val == 10){
    Entry entry = Browser::entries[active_gallery];
    printf("URL %s\n", entry.url.c_str());
    GalleryBrowser::set_touch();
    GalleryBrowser::load_gallery(&entry);
    return Handler::Gallery;
  } else if (val == 11) {
    SearchBrowser::set_touch();
    return Handler::Search;
  }

  return Handler::Browser;
}

void Browser::render_entry(Entry* entry, int x, int y, bool active)
{
  // If image not loaded, stick in texture
  if(entry->thumb_loaded == 0){
    printf("Loading texture into memory\n");
    //entry->res = new Resource();
    entry->res->url = entry->thumb;
    ApiManager::request_res(entry->res);
    entry->thumb_loaded = 1;
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

  if(!entry->thumb_texture){
    if(entry->res->mem->size != 0){
      entry->thumb_texture = Screen::load_texture(entry->res->mem->memory, entry->res->mem->size);
      delete entry->res->mem;
    }
  }

  if(entry->thumb_texture){
    Screen::draw_adjusted_mem(entry->thumb_texture, x, y, maxw, maxh);
  }

  Screen::draw_text(new_title, x + maxw + 10, y + 5, ThemeText, Screen::gallery_info);
  Screen::draw_text(entry->category, x + maxw + 10, y + 30, ThemeText, Screen::gallery_info);
  Screen::draw_text(std::to_string(entry->pages).c_str(), x + maxw + 10, y + 50, ThemeText, Screen::gallery_info);
  Screen::draw_text("Pages", x + maxw + 70, y + 50, ThemeText, Screen::gallery_info);
}
