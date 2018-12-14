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

#include "Ui.hpp"

struct MemoryStruct;
struct Tag;
struct Resource;
struct Entry;

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
		static json_object* post_api(char* payload, std::string url);
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

struct Resource{
  MemoryStruct* mem;
  SDL_Texture* texture;
  std::string url;
  int meta;
  void(*res_func)(Resource*);

  int done;
  int requested;
  int populated;
  Resource(){
    texture = Screen::s_loading;
    mem = new MemoryStruct();
    done = 0;
    requested = 0;
    populated = 0;
  }
  ~Resource(){
    if(texture)
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