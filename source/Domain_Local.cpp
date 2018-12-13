#include "Domain.hpp"
#include <filesystem>
#include <fstream>

#define app_path "/switch/Reader/"

void Domain_Local::search(std::string keywords){
  Domain::search(keywords);

  std::vector<std::string> gallery_dirs = get_directories(app_path);

  for(std::string path : gallery_dirs){
    printf("Loading local %s\n", path.c_str());
    // Load info to Entry
    std::ifstream info (path + "/info");
    std::string line;

    Entry* entry = new Entry();
    getline(info, entry->title);
    getline(info, entry->category);
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