#include "Domain.hpp"
#include "Config.hpp"
#include "Api_Szuru.hpp"
#include "Base64.hpp"
#include "Touch.hpp"
#include "Keyboard.hpp"
#include "HSearch.hpp"
#include "Preview.hpp"

#include <unistd.h>

void Domain_Szuru::login(std::string username, std::string password){
  printf("Checking user token\n");
  // Check if token exists
  std::string token = ConfigManager::get_value("szuru_token_" + name);

  printf("Current token : %s\n", token.c_str());
  
  // Check token is working
  bool valid = Api_Szuru::check_token(token);

  // If valid stop now, else login
  if(!valid){
    printf("Logging in...\n");
    token = Api_Szuru::update_token(username, password);
  } else {
    printf("Token already valid\n");
    return;
  }

  // Failed to auth
  if(token.empty()){
    return;
  }

  printf("Returned Token : %s\n", token.c_str());

  // Generate Base64 auth token
  std::string encoded = base64_encode((const unsigned char*) (username + ":" + token).c_str(), username.length() + 1 + token.length());
  printf("Encoded Token : %s\n", encoded.c_str());

  ConfigManager::set_pair("szuru_token_" + name, encoded);
  this->token = encoded;

}

void Domain_Szuru::empty_search(){
  std::multimap<std::string, std::string> configPairs = ConfigManager::get_all();
  std::string check = "szuru_search_" + name;
  printf("Checker %s\n", check.c_str());
  Browser::numOfResults = 0;
  json_object* result;
  json_object* holder;

  // All files
  result = Api_Szuru::list_posts(token, 0, 1, "");

  // Failure
  if(result == NULL){
    if(get_json_obj(result, "query") == NULL){
      json_object_put(result);
      return;
    } else {
      return;
    }
  }

  // Build result
  Entry* entry = new Entry();
  entry->title = "All Posts";
  entry->url = "";
  entry->pages = json_object_get_int(get_json_obj(result, "total"));
  
  // Add thumbnail, if result given
  if(entry->pages != 0){
    holder = get_json_obj(result, "results");
    holder = json_object_array_get_idx(holder, 0);
    entry->thumb = domain + json_object_get_string(get_json_obj(holder, "thumbnailUrl"));
  } else {
    entry->thumb = "empty";
  }

  // Add to browser
  Browser::add_entry(entry);

  json_object_put(result);

  // Get all searches
  for(const auto &pair : configPairs){
    if(pair.first == check){

      // Extract name and query
      auto delimiterPos = pair.second.find(",");
      auto name = pair.second.substr(0, delimiterPos);
      auto query = pair.second.substr(delimiterPos + 1);
      
      printf("Requesting\n");
      // Pull thumbnails for searches
      result = Api_Szuru::list_posts(token, 0, 1, query);

      // Failure
      if(result == NULL){
        continue;
      } else if(get_json_obj(result, "query") == NULL){
        json_object_put(result);
        continue;
      }

      // Build result
      Entry* entry = new Entry();
      entry->title = name;
      entry->url = query;
      entry->pages = json_object_get_int(get_json_obj(result, "total"));
      
      // Add thumbnail, if result given
      if(entry->pages != 0){
        holder = get_json_obj(result, "results");
        holder = json_object_array_get_idx(holder, 0);
        entry->thumb = domain + json_object_get_string(get_json_obj(holder, "thumbnailUrl"));
      } else {
        entry->thumb = "empty";
      }

      // Add to browser
      Browser::add_entry(entry);

      json_object_put(result);
    }
  } 
}

void Domain_Szuru::search(std::string keywords){
  // Clear up old search
  if(search_entry != nullptr)
    delete search_entry;

  if(!token_valid()){
    return;
  }

  if(keywords.empty()){
    empty_search();
  } else {
    printf("Manual search\n");
    json_object* json = Api_Szuru::list_posts(token, 0, 1, keywords);
    
    if(json != NULL){
      printf("Gotten result\n");
      // Successful search
      if(get_json_obj(json, "query") != NULL){
        printf("Success\n");
        search_entry = new Entry();
        search_entry->title = keywords;
        search_entry->url = keywords;
        search_entry->pages = json_object_get_int(get_json_obj(json, "total"));

        // No results!
        if(search_entry->pages == 0){
          printf("No results!\n");
          delete search_entry;

          // Notify user
          Screen::clear(ThemeBG);
          Screen::draw_text_centered("No Results", 0, 0, screen_width, screen_height, ThemeText, Screen::header);
          Screen::render();
          sleep(2);

          return;
        }

        json_object* holder = get_json_obj(json, "results");
        holder = json_object_array_get_idx(holder, 0);
        search_entry->thumb = domain + json_object_get_string(get_json_obj(holder, "thumbnailUrl"));

        // Set up gallery
        GalleryPreview::load_gallery(search_entry);
        GalleryPreview::set_touch();
        Browser::force_handler(HandlerEnum::Preview);
      }

      json_object_put(json);
    }

    // Load Search as Gallery

    // Force into Gallery Preview mode
  }
}

bool Domain_Szuru::token_valid(){

  // Load token
  if(token.empty()){
    token = ConfigManager::get_value("szuru_token_" + name);

    // No token saved
    if(token.empty())
      return false;
  }

  printf("Checking %s\n", token.c_str());

  // Check if current token is still active
  bool success = Api_Szuru::check_token(token);
  if(success)
    username = "Browsing Booru";
  else
    username = "Not Logged In";

  Browser::load_username();

  return success;
}


void Domain_Szuru::prefill_gallery(Entry* e, Gallery* gallery){
  json_object* json;
  json_object* holder;
  json_object* result;
  int pages = 0;
  std::string query = e->url;

  if(ConfigManager::get_value("szuru_random") == "1")
    query += " sort:random";

  json = Api_Szuru::list_posts(token, 0, 100, query);  
  holder = get_json_obj(json, "results");
  pages = json_object_array_length(holder);
  for(int i = 0; i < pages; i++){
    result = json_object_array_get_idx(holder, i);

    Resource* res = new Resource();
    res->url = domain + json_object_get_string(get_json_obj(result, "contentUrl"));
    res->populated = 1;
    gallery->images.push_back(res);
  }

  // 100 pages per block
  GalleryBrowser::block_size = 100;

}

void Domain_Szuru::load_gallery_urls(size_t page, int* block_size, Gallery* gallery){
  json_object* json;
  json_object* holder;
  json_object* result;
  int pages = 0;
  int offset = (gallery->images.size() / *block_size) * 100;
  std::string query = gallery->entry->url;

  if(ConfigManager::get_value("szuru_random") == "1")
    query += " sort:random";

  json = Api_Szuru::list_posts(token, offset, 100, query); 
  holder = get_json_obj(json, "results");
  pages = json_object_array_length(holder);
  for(int i = 0; i < pages; i++){
    result = json_object_array_get_idx(holder, i);

    Resource* res = new Resource();
    res->url = domain + json_object_get_string(get_json_obj(result, "contentUrl"));
    res->populated = 1;
    gallery->images.push_back(res);
  }

  json_object_put(json);
}

void Domain_Szuru::browser_touch(){
  // List Queries
  TouchManager::instance.add_bounds(screen_width - 190, 395, 180, 80, 151);

  // Add Query
  TouchManager::instance.add_bounds(screen_width - 190, 495, 180, 80, 150);
}

void Domain_Szuru::browser_render(){
  // List Queries
  Screen::draw_button(screen_width - 190, 395, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_text_centered("All Query", screen_width - 190, 395, 180, 80, ThemeButtonText, Screen::normal);

  // Add Query
  Screen::draw_button(screen_width - 190, 495, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Add Query", screen_width - 190, 495, 180, 80, ThemeButtonText, Screen::normal);
}

HandlerEnum Domain_Szuru::browser_event(int val){
  std::string name;
  std::string query;

  switch(val){
    case 150:
      name = Keyboard::get_input("Name");
      query = Keyboard::get_input("Query");
      ConfigManager::add_pair("szuru_search_" + this->name, name + "," + query);
      Browser::clear();
      HSearch::search_keywords("");
      break;
    case 151:
      Browser::clear();
      HSearch::search_keywords("");
      break;
    default:
      break;
  }

  return HandlerEnum::Browser;
}

void Domain_Szuru::preview_touch(){
  // Remove query
  TouchManager::instance.add_bounds(screen_width-190, screen_height - 100, 180, 80, 50);

  // Random
  TouchManager::instance.add_bounds(screen_width-100, 395, 180, 80, 51);
}

void Domain_Szuru::preview_render(){
  // Remove query button
  Screen::draw_button(screen_width-190, screen_height - 100, 180, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Delete", screen_width-190, screen_height - 100, 180, 80, ThemeButtonText, Screen::normal);

  // Random button
  if(ConfigManager::get_value("szuru_random") == "1")
    Screen::draw_button(screen_width-190, 395, 180, 80, ThemeOptionSelected, ThemeButtonBorder, 4);
  else
    Screen::draw_button(screen_width-190, 395, 180, 80, ThemeOptionUnselected, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Random", screen_width-190, 395, 180, 80, ThemeButtonText, Screen::normal);
}

HandlerEnum Domain_Szuru::preview_event(int val){
  Entry* e;
  std::string toggle;

  switch(val){
    // Remove query
    case 50:
      e = GalleryPreview::entry;
      ConfigManager::remove_pair("szuru_search_" + name, e->title + "," + e->url);
      Browser::clear();
      HSearch::search_keywords("");
      return HandlerEnum::Browser;
      break;
    // Toggle random
    case 51:
      toggle = ConfigManager::get_value("szuru_random");
      if(toggle == "1")
        toggle = "0";
      else
        toggle = "1";
      ConfigManager::set_pair("szuru_random", toggle);
      break;
    default:
      break;
  }

  return HandlerEnum::Preview;
}

std::string Domain_Szuru::get_username(){
  return username;
}