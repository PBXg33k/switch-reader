#include "ui.h"
#include "api.h"
#include "Browser.h"
#include "shared.h"
#include "TouchManager.h"

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
      SDL_Rect rect = {newX, newY, newX + Browser::maxw2, newY + Browser::maxh};
      TouchManager::add_bounds(rect, val);
      val++;
    }
  }

  SDL_Rect button = {screen_width-60, (screen_height/25) - 25, 50, 50};
  TouchManager::add_bounds(button, 10);
}

void Browser::new_entry(Entry* entry, json_object* json, int num)
{
  json_object *holder;

  // Populate entry
  json = get_json_obj(json, "gmetadata");
  json = json_object_array_get_idx(json, num);

  holder = get_json_obj(json, "title");
  entry->title = json_object_get_string(holder);

  holder = get_json_obj(json, "category");
  entry->category = json_object_get_string(holder);

  holder = get_json_obj(json, "thumb");
  entry->thumb = json_object_get_string(holder);

  entry->thumb_loaded = 0;
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
  printf("Rendering %d Entries\n", num_entries);


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
  Screen::draw_rect(screen_width - 60, (screen_height/2) - 25, 50, 50, COLOR_WHITE);
  Screen::draw_text("Next", screen_width - 55, (screen_height/2)-45, COLOR_BLACK, Screen::gallery_info);
}

void Browser::on_event(int val){
  if(val >= 0){
    Browser::active_gallery = val;
  }
}

void Browser::render_entry(Entry* entry, int x, int y, bool active)
{
  // If image not loaded, stick in texture
  if(entry->thumb_loaded == 0){
    printf("Loading texture into memory\n");
    MemoryStruct thumb_data = ApiManager::get_res((char*) entry->thumb);
    entry->thumb_data = thumb_data;
    entry->thumb_texture = Screen::load_texture(thumb_data.memory, thumb_data.size);
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

  MemoryStruct* thumb_data = &(entry->thumb_data);
  Screen::draw_adjusted_mem(entry->thumb_texture, x, y, maxw, maxh);
  Screen::draw_text(new_title, x + maxw + 10, y + 5, COLOR_WHITE, Screen::gallery_info);
  Screen::draw_text(entry->category, x + maxw + 10, y + 30, COLOR_WHITE, Screen::gallery_info);
}
