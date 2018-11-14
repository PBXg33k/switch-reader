#pragma once

#include <curl/curl.h>
#include <json-c/json.h>
#include <string>
#include <vector>
#include <mutex>
#include <switch.h>
#include <SDL2/SDL.h>

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

struct Tag{
  std::string category;
  std::string tag;
  Tag(std::string c, std::string t){
    category = c;
    tag = t;
  }
};

struct Resource{
  MemoryStruct* mem;
  SDL_Texture* texture;
  std::string url;
  int done;
  int requested;
  int populated;
  Resource(){
    texture = NULL;
    mem = new MemoryStruct();
    done = 0;
    requested = 0;
    populated = 0;
  }
  ~Resource(){
    if(texture)
      SDL_DestroyTexture(texture);
    delete mem;
  }
};

struct Entry{
  int id;
  std::string title;
  std::string category;
  std::string language;
  double rating;
  std::string thumb;
  std::string url;
  int pages;
  std::vector<Tag> tags;
  int local;

  Resource* res;
  Entry(){
    local = 0;
    res = new Resource();
  }
  ~Entry(){
    delete res;
  }
};

class ApiManager {
	public:
		static void init();
		static void close();
    static void update();
		static void api_test();
    static void cleanup_resource(Resource* res);
    static void cancel_all_requests();
    static void request_res(Resource* res);
		static void get_res(MemoryStruct* mem, std::string url, int save=0, std::string path = std::string());
		static json_object* post_api(char* payload);
    static void download_gallery(Entry* entry, float* percent);

    static const std::string gallery_template;

		static json_object* get_galleries(std::vector<std::string> gids, std::vector<std::string> gtkns);
};