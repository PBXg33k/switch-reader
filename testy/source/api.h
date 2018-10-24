#ifndef API_H
#define API_H

#include <curl/curl.h>
#include <json-c/json.h>
#include <string>

typedef struct MemoryStruct {
  char *memory;
  size_t size;
}MemoryStruct;

class ApiManager {
	public:
		static void init();
		static void close();
		static void api_test();
		static struct MemoryStruct get_res(const char* url);
		static json_object* post_api(char* payload);

		static json_object* get_gallery(const char* g_id, const char* g_token);
};

#endif
