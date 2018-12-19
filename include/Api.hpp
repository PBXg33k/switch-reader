#pragma once

#include <curl/curl.h>
#include <json-c/json.h>
#include <string>
#include <vector>
#include <mutex>
#include <switch.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <map>

#include "SDL_Gifwrap.h"
#include "Ui.hpp"

struct MemoryStruct;
struct Tag;
struct Resource;
struct Entry;
struct GIF_Info;

class ApiManager {
	public:
		static void init();
		static void close();
    static void update();
		static void api_test();
    static void cleanup_resource(Resource* res);
    static void cancel_all_requests();
    static void handle_req(Resource* res);
    static void request_res(Resource* res, void(*res_func)(Resource*) = handle_req);

		static void get_res(MemoryStruct* mem, std::string url, CURL* curl=ApiManager::handle, int save=0, std::string path = std::string());
    static void load_res(MemoryStruct* mem, std::string url);
    static json_object* get_res_json(std::string url, CURL* curl);
		static json_object* post_api(const char* payload, std::string url, CURL* curl);

    static int download_gallery(Entry* entry);

    static const std::string gallery_template;
    static bool delete_active;

    static CURL* thread_handle;
    static CURL* handle;
};

struct MemoryStruct {
  char *memory;
  size_t size;
  MemoryStruct(){
    memory = new char;
    size = 0;
  }
  ~MemoryStruct(){
    delete memory;
  }
};

struct GIF_Info{
  GIF_Image* gif;
  int frame;
  SDL_Texture* texture;
  unsigned long next_render;
  unsigned long delay;
  std::string temp_path;

  SDL_Texture* get_texture();
  void load_gif(MemoryStruct* mem);

  GIF_Info(MemoryStruct* mem) : GIF_Info() {
    load_gif(mem);
  }

  GIF_Info(){
    next_render = 0L;
    frame = 0;
    delay = 0L;
    gif = nullptr;
    texture = nullptr;
  }

  ~GIF_Info(){
    if(texture != nullptr)
      SDL_DestroyTexture(texture);
    if(gif != nullptr)
      GIF_FreeImage(gif);
  }
};


struct Resource{
  MemoryStruct* mem;
  SDL_Texture* texture;
  GIF_Info* gif;
  std::string url;
  int meta;
  void(*res_func)(Resource*);

  bool is_gif;
  int done;
  int requested;
  int populated;
  Resource(){
    texture = Screen::s_loading;
    mem = new MemoryStruct();
    done = 0;
    requested = 0;
    populated = 0;
    is_gif = false;
    gif = nullptr;
  }
  ~Resource(){
    if(gif)
      delete gif;
    // Gif deletes texture, don't do it twice
    else if(texture)
      Screen::cleanup_texture(texture);
    delete mem;
  }
};

struct Entry{
  int id;
  int media_id;
  std::string title;
  std::string category;
  std::string language;
  double rating;
  std::string thumb;
  std::string url;
  int pages;
  std::multimap<std::string, std::string> tags;
  int local;
  Resource* res;
  
  Entry(){
    local = 0;
    media_id = 0;
    res = new Resource();
  }
  ~Entry(){
    ApiManager::cleanup_resource(res);
  }

  friend std::ostream& operator<<(std::ostream& os, const Entry& entry){
    os << entry.title << "\n";
    os << entry.category << "\n";
    os << entry.language << "\n";
    os << entry.rating << "\n";
    os << entry.pages << "\n";
    for(auto tag : entry.tags)
      os << tag.first << ":" << tag.second << "\n";
    return os;
  };
};