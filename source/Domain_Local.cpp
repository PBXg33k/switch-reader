#include "Domain.hpp"
#include "Config.hpp"
#include "Touch.hpp"
#include "Preview.hpp"
#include "HSearch.hpp"

#include <filesystem>
#include <fstream>

void Domain_Local::search(std::string keywords){
  Browser::numOfResults = 0;

  std::vector<std::string> gallery_dirs = get_directories(ConfigManager::downloadsDir);

  for(std::string path : gallery_dirs){
    printf("Loading local %s\n", path.c_str());
    // Load info to Entry
    std::ifstream info (path + "/info");
    std::string line;

    Entry* entry = new Entry();
    entry->url = path;
    getline(info, entry->title);
    getline(info, entry->category);
    getline(info, entry->language);
    getline(info, line);
    entry->rating = atof(line.c_str());
    getline(info, line);
    entry->pages = stoi(line);

    // Tags
    while(getline(info, line)){
      auto delimiterPos = line.find(":");
      auto type = line.substr(0, delimiterPos);
      auto name = line.substr(delimiterPos + 1);

      entry->tags.insert(std::make_pair(type, name));
    }

    entry->url = path;
    entry->thumb = path + "/thumb.jpg";

    Browser::add_entry(entry);

    info.close();
  }

}

void Domain_Local::prefill_gallery(Entry* e, Gallery* gallery){
  for(int i = 0; i < gallery->total_pages; i++){
    Resource* res = new Resource();
    res->populated = 1;
    res->url = gallery->index + "/page" + std::to_string(i) + ".jpg";
    gallery->images.push_back(res);
  }
}

std::vector<std::string> Domain_Local::get_directories(const std::string& s)
{
  std::vector<std::string> r;
  for(auto& p : std::filesystem::recursive_directory_iterator(s))
    if(p.status().type() == std::filesystem::file_type::directory)
      r.push_back(p.path().string());
  return r;
}

void Domain_Local::preview_touch(){
  // Remove gallery
  TouchManager::instance.add_bounds(screen_width-190, screen_height - 100, 180, 80, 50);
}

void Domain_Local::preview_render(){
  // Remove gallery button
  Screen::draw_button(screen_width-190, screen_height - 100, 180, 80, ThemeOptionSelected, ThemeButtonBorder, 4);
  Screen::draw_text_centered("Delete", screen_width-190, screen_height - 100, 180, 80, ThemeButtonText, Screen::normal);
}

HandlerEnum Domain_Local::preview_event(int val){
  // // Remove gallery
  // if(val == 50){
  //   printf("Removing %s\n", GalleryPreview::entry->url.c_str());
  //   std::filesystem::remove_all(GalleryPreview::entry->url.c_str());
  //   Browser::clear();
  //   HSearch::search_keywords("");
  //   return HandlerEnum::Browser;
  // }

  return HandlerEnum::Preview;
}