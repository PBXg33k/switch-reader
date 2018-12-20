#include "Domain.hpp"
#include "Config.hpp"
#include "Api_Lanraragi.hpp"
#include "Touch.hpp"
#include "HSearch.hpp"
#include "Dialog.hpp"

#include <sstream>

void Domain_Lanraragi::search(std::string keywords){
  // Check api key is valid
  if(!key_valid())
    return;

  json_object* json = Api_Lanraragi::list_posts(api_key);
  json_object* holder;
  int total = json_object_array_length(json);
  for(int i = 0; i < total; i++){
    holder = json_object_array_get_idx(json, i);

    Entry* entry = new Entry;
    entry->title = json_object_get_string(get_json_obj(holder, "title"));
    entry->url = domain + "api/extract?key=" + api_key + "&id=" + json_object_get_string(get_json_obj(holder, "arcid"));
    entry->thumb = domain + "api/thumbnail?key=" + api_key + "&id=" + json_object_get_string(get_json_obj(holder, "arcid"));

    // List tags string into list
    std::string tags = json_object_get_string(get_json_obj(holder, "tags"));
    std::vector<std::string> tags_list;
    std::istringstream iss(tags);
    copy(std::istream_iterator<std::string>(iss),
              std::istream_iterator<std::string>(),
              back_inserter(tags_list));

    for(auto tag : tags_list){
      // Split category:tag
      auto delimiterPos = tag.find(":");
      std::string category;
      std::string tag_name;
      if(delimiterPos != std::string::npos){
        category = tag.substr(0, delimiterPos);
        tag_name = tag.substr(delimiterPos + 1);
      } else {
        category = "Misc";
        tag_name = tag;
      }
      
      entry->tags.insert(std::make_pair(category, tag_name));
    }

    entry->pages = -1;

    Browser::add_entry(entry);
  }

  if(json != NULL)
    json_object_put(json);
}

void Domain_Lanraragi::prefill_gallery(Entry* entry, Gallery* gallery){
  json_object* json = ApiManager::get_res_json(entry->url, ApiManager::handle);
  json_object* pages = get_json_obj(json, "pages");
  int total = json_object_array_length(pages);
  entry->pages = total;

  // Split up page urls and store
  for(int i = 0; i < total; i++){
    std::string page = json_object_get_string(json_object_array_get_idx(pages, i));
    page = page.substr(2);
    Resource* res = new Resource();
    res->url = domain + page;
    res->populated = 1;
    printf("Page %s\n", res->url);
    gallery->images.push_back(res);
  }

  json_object_put(json);


}

bool Domain_Lanraragi::key_valid(){
  // Get key if not set
  if(api_key.empty()){
    api_key = ConfigManager::get_value("lanraragi_key_" + name);
  }

  // No key, no entry
  if(api_key.empty()) {
    return false;
  // Verify connection
  } else {
    return Api_Lanraragi::check_key(api_key);
  }

  return false;
}

void Domain_Lanraragi::login(std::string username, std::string password){
  api_key = username;
  ConfigManager::set_pair("lanraragi_key_" + name, api_key);
}

void Domain_Lanraragi::settings_touch(){
  // Remove Lanraragi
  TouchManager::instance.add_bounds(860, 330, 330, 120, 50);
}

void Domain_Lanraragi::settings_render(){
  // Remove Szuru
  Screen::draw_button(860, 330, 330, 120, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text_centered("Del Lanra. Domain", 860, 330, 330, 120, ThemeButtonText, Screen::large);
}

HandlerEnum Domain_Lanraragi::settings_event(int val){
  // Remove Lanraragi
  if(val == 50){
    // Build config value to remove
    Domain_Lanraragi* domain = (Domain_Lanraragi*) HSearch::current_domain();
    std::string val = domain->name + "," + domain->domain.c_str();

    // Verify act of deletion
    if(Dialog::get_bool("Are you sure you want to delete '" + name + "'?")){
      ConfigManager::remove_pair("lanraragi_domain", val);
      HSearch::delete_domain(domain->name);
      printf("Removed val %s\n", val.c_str());

      // Remove tokens ?

      // Set to first listed domain
      std::map<std::string, Domain*> list_of_domains = HSearch::get_domains();
      if(list_of_domains.size() > 0){
        ConfigManager::set_pair("domain", list_of_domains.begin()->first);
        printf("Set new domain %s\n", list_of_domains.begin()->first.c_str());
      }
    }
  }

  return HandlerEnum::Settings;
}
