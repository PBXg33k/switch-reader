#include "Api.hpp"
#include "Browser.hpp"
#include "Ui.hpp"
#include <iostream>
#include <cstring>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <deque>

static std::deque<Resource*> requests;
static std::vector<Resource*> cleanup;
static Resource* active_res = NULL;
static Thread* res_thread = NULL;
static Mutex* mutex = new Mutex;

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
  // Declare static mutex
  mutexInit(mutex);
  res_thread = new Thread();

  // Create render event

}

void ApiManager::close(){
  curl_global_cleanup();
  socketExit();
}

void ApiManager::cleanup_resource(Resource* res){
  if(active_res != res)
    delete res;
  else {
    mutexLock(mutex);
    delete res;
    active_res = NULL;
    mutexUnlock(mutex);
  }
}

void load_res_thread(void *args){
  Resource* res = (Resource*) args;

  mutexLock(mutex);
  ApiManager::get_res(res->mem, res->url);
  res->done = 1;
  mutexUnlock(mutex);
}

void ApiManager::update(){
  // If requests are waiting, start request thread
  if(active_res == NULL && !requests.empty()){

    // Ignore cancelled requests
    while(!requests.empty()){
      active_res = requests.front();
      requests.pop_front();

      if(active_res->requested)
        break;
    }

    // If all cancelled, set to none
    if(!active_res->requested){
      active_res = NULL;
      return;
    }

    threadCreate(res_thread, load_res_thread, active_res, 5000, 0x2C, -2);
    threadStart(res_thread);

  } else if (active_res != NULL){
    // Try and get lock on active_res
    if(mutexTryLock(mutex)){
      // If done, load texture into memory, close thread
      if(active_res->done){
        // If cancelled, don't load texture
        if(active_res->requested)
          active_res->texture = Screen::load_texture(active_res->mem->memory, active_res->mem->size);

        // Reset memory struct
        //delete active_res->mem;
        active_res->mem = new MemoryStruct();

        // Cleanup thread
        threadWaitForExit(res_thread);
        threadClose(res_thread);
        active_res = NULL;
      }

      mutexUnlock(mutex);
    }
  }
}

// Cancel all requests queued to be loaded request thread
void ApiManager::cancel_all_requests(){
  for(auto res : requests){
    res->requested = 0;
  }
  requests.clear();
}

// Threaded image loading
void ApiManager::request_res(Resource* res){
  // Active_res is loaded in thread - Don't touch done.
  if(active_res != res)
    res->done = 0;
  res->requested = 1;
  requests.push_back(res);

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
  const char *host = "http://35.205.50.155:5000/?url=";

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
    //printf("%zu bytes retrieved\n", chunk->size);
  }
  curl_global_cleanup();
}

json_object* ApiManager::post_api(char* payload)
{
  CURL *curl;
  std::string readBuffer;

  curl = curl_easy_init();

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://35.205.50.155:5000/");
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
  return NULL;
}
