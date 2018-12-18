#include "Domain.hpp"
#include "Config.hpp"

void Domain::process_gallery_req(Resource* res){}
void Domain::search(std::string keywords){ Browser::numOfResults = 0; }
void Domain::search(std::string keywords, std::string type){}
void Domain::expand_search(std::string completeURL, int page){}
void Domain::search_favourites(){}
void Domain::prefill_gallery(Entry* e, Gallery* gallery){}
void Domain::load_gallery_urls(size_t page, int* block_size, Gallery* gallery){}
int Domain::download_gallery(Gallery* gallery){ return 1; }
void Domain::login(std::string username, std::string password) {}
std::string Domain::get_username() { return ""; }
void Domain::download_update(int page, Gallery* gallery){
  int progress = ((float) page / (float) gallery->total_pages) * ((screen_width / 2) - 10);
  Screen::clear(ThemeBG);
  std::string to_print = "Downloading Gallery - Page " + std::to_string(page + 1) + " of " + std::to_string(gallery->images.size());
  Screen::draw_text_centered(to_print, 0, (screen_height / 2) - 120, screen_width, 100, ThemeText, Screen::header);
  Screen::draw_rect(screen_width / 4, screen_height / 2, screen_width / 2, 150, ThemePanelDark);
  Screen::draw_rect(screen_width / 4 + 5, (screen_height / 2) + 5, progress, 140, ThemePanelLight);
  Screen::render(); 
}

void Domain::search_touch(){}
void Domain::search_render(){}
HandlerEnum Domain::search_event(int val){ return HandlerEnum::Search; }

void Domain::browser_touch(){}
void Domain::browser_render(){}
HandlerEnum Domain::browser_event(int val){ return HandlerEnum::Browser; }

void Domain::settings_touch(){}
void Domain::settings_render(){}
HandlerEnum Domain::settings_event(int val){ return HandlerEnum::Settings; }

void Domain::preview_touch(){}
void Domain::preview_render(){}
HandlerEnum Domain::preview_event(int val){ return HandlerEnum::Preview; }

xmlXPathObjectPtr Domain::get_node_set(xmlDocPtr doc, xmlChar *xpath){
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;

    context = xmlXPathNewContext(doc);
    result = xmlXPathEvalExpression(xpath, context);
    return result;
}

json_object* Domain::get_json_obj(json_object* root, std::string key)
{
  json_object* ret;
  if (json_object_object_get_ex(root, key.c_str(), &ret)){
    return ret;
  }
  return NULL;
}