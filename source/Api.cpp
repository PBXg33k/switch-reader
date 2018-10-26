#include "Api.hpp"
#include <iostream>
#include <cstring>
#include <thread>
#include <mutex>

typedef struct Resource{
  MemoryStruct* mem;
  Mutex* mutex;
  std::string url;
}Resource;

static std::vector<Resource*> requests;

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  MemoryStruct *mem = (MemoryStruct *)userp;

  char *ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

void ApiManager::init(){
  socketInitializeDefault();
  curl_global_init(CURL_GLOBAL_ALL);
}

void ApiManager::close(){
  curl_global_cleanup();
  socketExit();
}

void prompt_request_queue(){}
// 
// void test_res(void *args){
//   Resource* res = (Resource*) args;
//
//   mutexLock(res->mutex);
//   ApiManager::get_res(res->mem, res->url);
//   mutexUnlock(res->mutex);
// }

// Pass MemoryStruct and Mutex - Can afford to wait
void ApiManager::request_res(MemoryStruct* mem, std::string url){
  printf("Requesting\n");
  //
  // // Create thread struct to hold gallery_info
  // Thread* resThread = new Thread();
  // Result result;
  //
  // //Create resource to Pass
  // Resource* res = new Resource();
  // res->mem = mem;
  // res->mutex = mutex;
  // res->url = url;
  //
  // threadCreate(resThread, test_res, res, 5000, 0x2C, -2);
  // threadStart(resThread);
  // result = threadWaitForExit(resThread);
  // threadClose(resThread);

  get_res(mem, url);
}

json_object* ApiManager::get_galleries(std::vector<std::string> gids, std::vector<std::string> gtkns){
  char* temp = (char *) "[%s,\"%s\"]";
  std::string gallery_list;
  size_t size;
  char* buffer;

  printf("Making string\n");

  for(size_t c = 0; c < gids.size(); c++){
    size = snprintf(NULL, 0, temp, gids[c].c_str(), gtkns[c].c_str());
    buffer = (char*) malloc(size + 1);
    snprintf(buffer, size + 1, temp, gids[c].c_str(), gtkns[c].c_str());
    gallery_list.append(buffer);
    gallery_list.append(",");
  }
  gallery_list.resize(gallery_list.size() - 1);

  char* data = (char*)malloc((strlen(gallery_list.c_str()) + 64) * sizeof(char));
  sprintf(data, "{\"method\": \"gdata\",\"gidlist\": [%s],\"namespace\": 1}", gallery_list.c_str());
  printf("%s\n",data);
  return ApiManager::post_api(data);
}

void ApiManager::api_test()
{
  CURL *curl;
  FILE *testfile;
  FILE *logfile;

  logfile = fopen("/switch/logtest.txt", "wb");

  curl = curl_easy_init();

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://barryis.fun:5000/");
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"method\": \"gdata\",\"gidlist\": [[618395,\"0439fa3666\"]],\"namespace\": 1}");
    curl_easy_setopt(curl, CURLOPT_STDERR, logfile);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

    testfile = fopen("/switch/testfile", "wb");

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, testfile);
  	curl_easy_perform(curl);
  	fclose(testfile);
    fclose(logfile);
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
}



void ApiManager::get_res(MemoryStruct* chunk, std::string url)
{
  CURL *curl;
  const char *host = "http://192.168.0.12:5000/?url=";

  curl = curl_easy_init();
  char *uri = curl_easy_escape(curl, url.c_str(), strlen(url.c_str()));

  char *link = (char*)malloc(strlen(host) + strlen(uri) + 1);
  strcpy(link, host);
  strcat(link, uri);

  if(curl) {
    //curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_URL, link);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
  	curl_easy_perform(curl);
    curl_free(uri);
    curl_easy_cleanup(curl);
    printf("%zu bytes retrieved\n", chunk->size);
    FILE *test = fopen("test2.jpg", "wb");
    fwrite(chunk->memory, 4, chunk->size, test);
    fclose(test);
  }
  curl_global_cleanup();
}

json_object* ApiManager::post_api(char* payload)
{
  CURL *curl;
  std::string readBuffer;

  curl = curl_easy_init();

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.0.12:5000/");
    //curl_easy_setopt(curl, CURLOPT_URL, "https://api.e-hentai.org/api.php");
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  	curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    // Mark json as in use - caller must put later
    json_object* json = json_tokener_parse(readBuffer.c_str());
    json_object_get(json);
    return json;
  }
  curl_global_cleanup();
  const char* empty = "";
  return json_tokener_parse(empty);
}
