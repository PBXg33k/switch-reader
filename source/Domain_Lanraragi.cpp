#include "Domain.hpp"
#include "Config.hpp"
#include "Api_Lanraragi.hpp"

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
    entry->url = domain + "api/extract?key" + api_key + "&id=" + json_object_get_string(get_json_obj(holder, "arcid"));
    entry->thumb = domain + "api/thumbnail?key" + api_key + "&id=" + json_object_get_string(get_json_obj(holder, "arcid"));
    printf("Set thumb %s\n", entry->url.c_str());
    entry->pages = -1;

    Browser::add_entry(entry);
  }
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