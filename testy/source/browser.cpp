#include "ui.h"
#include "api.h"
#include "browser.h"
#include "shared.h"

json_object* get_json_obj(json_object* root, const char* key)
{
  json_object* ret;
  if (json_object_object_get_ex(root, key, &ret)){
    return ret;
  }
  return NULL;
}

Entry Browser::add_entry(json_object* json, int num)
{
  json_object *holder;
  struct Entry entry;

  // Get entry by number
  json = get_json_obj(json, "gmetadata");
  json = json_object_array_get_idx(json, num);

  holder = get_json_obj(json, "title");
  entry.title = json_object_get_string(holder);

  holder = get_json_obj(json, "category");
  entry.category = json_object_get_string(holder);

  holder = get_json_obj(json, "thumb");
  entry.thumb = json_object_get_string(holder);

  return entry;
}

void Browser::render_entry(Entry entry, int x, int y)
{
  MemoryStruct thumb = ApiManager::get_res((char*)entry.thumb);

  Screen::draw_adjusted_mem(thumb.memory, thumb.size, x, y, maxw, maxh);
  Screen::draw_text(entry.title, x, maxh + 10, COLOR_WHITE, Screen::gallery_info);
  Screen::draw_text(entry.category, x, maxh + 40, COLOR_WHITE, Screen::gallery_info);
}
