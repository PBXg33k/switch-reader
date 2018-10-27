#ifndef API_HPP
#define API_HPP

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
    memory = (char *) malloc(1);
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
  int done;
  int requested;
  Resource(){
    texture = NULL;
    mem = new MemoryStruct();
    done = 0;
    requested = 0;
  }
};

class ApiManager {
	public:
		static void init();
		static void close();
    static void update();
		static void api_test();
    static void cancel_all_requests();
    static void request_res(Resource* res);
		static void get_res(MemoryStruct* mem, std::string url);
		static json_object* post_api(char* payload);

    static const std::string gallery_template;

		static json_object* get_galleries(std::vector<std::string> gids, std::vector<std::string> gtkns);
};

#endif
