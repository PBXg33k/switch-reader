#include "Ui.hpp"
#include "Api.hpp"
#include "Browser.hpp"
#include "TouchManager.hpp"
#include "Gallery.hpp"

int Browser::grid_start = 0;
int Browser::active_gallery = -1;
std::vector<Entry> Browser::entries = std::vector<Entry>();

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

  // Next
  TouchManager::add_bounds(screen_width-190, (screen_height/2) - 40, 180, 80, 10);
  // Quit
  TouchManager::add_bounds(screen_width - 75, 0, 75, 75, 100);
}

Entry Browser::new_entry(json_object* json, int num)
{
  json_object *holder;
  struct Entry entry;

  // Populate entry
  json = get_json_obj(json, "gmetadata");
  json = json_object_array_get_idx(json, num);

  holder = get_json_obj(json, "title");
  entry.title = json_object_get_string(holder);

  holder = get_json_obj(json, "category");
  entry.category = json_object_get_string(holder);

  holder = get_json_obj(json, "thumb");
  entry.thumb = json_object_get_string(holder);

  holder = get_json_obj(json, "filecount");
  entry.pages = json_object_get_int(holder);

  entry.thumb_loaded = 0;
  return entry;
}

// Add entry to list of entries
void Browser::add_entry(Entry entry){
  entry.mutex = new Mutex();
  mutexInit(entry.mutex);
  Browser::entries.push_back(entry);
}

void Browser::clear(){
  Browser::entries.clear();
}

void Browser::render(){

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
      Browser::render_entry(entry, baseX + (x * incX), baseY + (y * incY), active);
      grid++;
    }
  }

  // Render next button
  Screen::draw_rect(screen_width-190, (screen_height/2) - 40, 180, 80, COLOR_WHITE);
  // Quit button
  Screen::draw_rect(screen_width - 75, 0, 75, 75, COLOR_RED);
  Screen::draw_text("Next", screen_width - 130, (screen_height/2)-10, COLOR_BLACK, Screen::normal);
}

Handler Browser::on_event(int val){
  if(val >= 0 && val < 10){
    Browser::active_gallery = val;
  }
  if(Browser::active_gallery >= 0 && val == 10){
    Entry entry = Browser::entries[active_gallery];
    printf("URL %s\n", entry.url.c_str());
    GalleryBrowser::set_touch();
    GalleryBrowser::load_gallery(&entry);
    Screen::clear(COLOR_BLACK);
    return Handler::Gallery;
  }

  return Handler::Browser;
}

void Browser::render_entry(Entry* entry, int x, int y, bool active)
{
  // If image not loaded, stick in texture
  if(entry->thumb_loaded == 0){
    printf("Loading texture into memory\n");
    entry->thumb_data = new MemoryStruct();
    ApiManager::request_res(entry->thumb_data, entry->mutex, entry->thumb);
    entry->thumb_texture = Screen::load_texture(entry->thumb_data->memory, entry->thumb_data->size);
    entry->thumb_loaded = 1;
  }

  std::string new_title = entry->title;
  new_title.resize(25);

  SDL_Color imgBG = COLOR_LIGHTGRAY;
  SDL_Color imgFG = COLOR_GRAY;
  if(active){
    imgBG = COLOR_DARKRED;
    imgFG = COLOR_RED;
  }

  Screen::draw_rect(x-5, y-5, maxw+10, maxh+10, imgBG);
  Screen::draw_rect(x + maxw+5, y-5, maxw+65, maxh+10, imgFG);

  // Lock texture before drawing - Might be loading!
  if(mutexTryLock(entry->mutex)){
    Screen::draw_adjusted_mem(entry->thumb_texture, x, y, maxw, maxh);
    mutexUnlock(entry->mutex);
  }

  Screen::draw_text(new_title, x + maxw + 10, y + 5, COLOR_WHITE, Screen::gallery_info);
  Screen::draw_text(entry->category, x + maxw + 10, y + 30, COLOR_WHITE, Screen::gallery_info);
  Screen::draw_text(std::to_string(entry->pages).c_str(), x + maxw + 10, y + 50, COLOR_WHITE, Screen::gallery_info);
  Screen::draw_text("Pages", x + maxw + 70, y + 50, COLOR_WHITE, Screen::gallery_info);
}
