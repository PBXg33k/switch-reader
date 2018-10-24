#include "ui.h"
#include "api.h"
#include "Browser.h"
#include "shared.h"
#include "TouchManager.h"

int Browser::grid_start = 0;
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
      SDL_Rect rect = {newX, newY, newX + Browser::maxw2, newY + Browser::maxh};
      TouchManager::add_bounds(rect, val);
      val++;
    }
  }
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

  entry.thumb_loaded = 0;
  return entry;
}

// Add entry to list of entries
void Browser::add_entry(Entry entry){
  Browser::entries.push_back(entry);
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
    for(int y = 0; (y < 3) && (grid <= num_entries); y++){
      Entry* entry = &(Browser::entries[grid]);
      Browser::render_entry(entry, baseX + (x * incX), baseY + (y * incY));
      grid++;
    }
  }
}

void Browser::render_entry(Entry* entry, int x, int y)
{
  if(entry->thumb_loaded == 0){
    entry->thumb_data = ApiManager::get_res((char*) entry->thumb);
    entry->thumb_loaded = 1;
  }

  MemoryStruct thumb = entry->thumb_data;
  std::string new_title = entry->title;
  new_title.resize(25);

  Screen::draw_rect(x-5, y-5, maxw+10, maxh+10, COLOR_LIGHTGRAY);
  Screen::draw_rect(x + maxw+5, y-5, maxw+65, maxh+10, COLOR_GRAY);
  Screen::draw_adjusted_mem(thumb.memory, thumb.size, x, y, maxw, maxh);
  Screen::draw_text(new_title, x + maxw + 10, y + 5, COLOR_WHITE, Screen::gallery_info);
  Screen::draw_text(entry->category, x + maxw + 10, y + 30, COLOR_WHITE, Screen::gallery_info);
}
