#include "Api.hpp"
#include "Browser.hpp"
#include "Gallery.hpp"
#include "Config.hpp"
#include "Shared.hpp"

#include <iostream>
#include <cstring>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <deque>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

static std::deque<Resource*> requests;
static std::vector<Resource*> cleanup;
static Resource* active_res = NULL;
static Thread* res_thread = NULL;
static Mutex* mutex = new Mutex;
static Mutex* cookieMutex = new Mutex;
static CURLSH* shared;

bool ApiManager::delete_active = false;

CURL* ApiManager::thread_handle;
CURL* ApiManager::handle;

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

static void lock_cb(CURL* curl, curl_lock_data data, curl_lock_access access, void *userptr){
  mutexLock(cookieMutex);
}

static void unlock_cb(CURL* curl, curl_lock_data data, void *userptr){
  mutexUnlock(cookieMutex);
}

void ApiManager::init(){

  socketInitializeDefault();
  nxlinkStdio();

  printf("Initializing API Manager...\n");
  curl_global_init(CURL_GLOBAL_ALL);
  // Declare static mutex
  mutexInit(mutex);
  mutexInit(cookieMutex);
  res_thread = new Thread();

  handle = curl_easy_init();
  thread_handle = curl_easy_init();

  shared = curl_share_init();
  curl_share_setopt(shared, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
  curl_share_setopt(shared, CURLSHOPT_LOCKFUNC, lock_cb);
  curl_share_setopt(shared, CURLSHOPT_UNLOCKFUNC, unlock_cb);

  curl_easy_setopt(handle, CURLOPT_SHARE, shared);
  curl_easy_setopt(thread_handle, CURLOPT_SHARE, shared);

  curl_easy_setopt(handle, CURLOPT_COOKIEFILE, "/switch/Reader/cookies");
  curl_easy_setopt(handle, CURLOPT_COOKIELIST, "RELOAD");
  curl_easy_perform(handle);
  curl_easy_reset(handle);

  printf("Loaded Cookies\n");
  struct curl_slist *cookies;
  curl_easy_getinfo(handle, CURLINFO_COOKIELIST, &cookies);
  while(cookies != nullptr){
    printf("%s\n", cookies->data);
    cookies = cookies->next;
  }
  curl_slist_free_all(cookies);

}

void ApiManager::close(){
  if(active_res != nullptr){
    printf("Closing open thread\n");
    threadWaitForExit(res_thread);
    threadClose(res_thread);
  } else {
    printf("No thread open\n");
  }

  printf("Flushing cookies\n");

  curl_easy_setopt(handle, CURLOPT_COOKIEJAR, "/switch/Reader/cookies");
  curl_easy_setopt(handle, CURLOPT_COOKIELIST, "FLUSH");
  curl_easy_perform(handle);

  printf("Curl cleanup\n");

  curl_global_cleanup();

  printf("Socket close\n");
  socketExit();
}

void ApiManager::cleanup_resource(Resource* res){
  if(active_res != res){
    requests.erase(std::remove(requests.begin(), requests.end(), res), requests.end());
    delete res;
  } else {
    // Queue to be cleaned up next update - Prevents hangs
    active_res->requested = 0;
    delete_active = true;
  }
}

void ApiManager::load_res(MemoryStruct* mem, std::string url){
  std::ifstream image (url);

  // Get size
  image.seekg(0, image.end);
  mem->size = image.tellg();
  image.seekg(0, image.beg);

  // Resize and load in data
  mem->memory = (char*) realloc(mem->memory, mem->size);
  image.read(mem->memory, mem->size);

  image.close();
}

void load_res_thread(void *args){
  Resource* res = (Resource*) args;

  mutexLock(mutex);

  // Url, use curl
  if(res->url.find("http") == 0)
    ApiManager::get_res(res->mem, res->url, ApiManager::thread_handle);
  // Local file, load normally
  else
    ApiManager::load_res(res->mem, res->url);

  res->done = 1;
  mutexUnlock(mutex);
}

int ApiManager::download_gallery(Entry* entry){
  // Save pages
  printf("Loading Gallery\n");
  GalleryBrowser::load_gallery(entry);
  printf("Saving\n");
  return GalleryBrowser::save_all_pages();
}

void ApiManager::handle_req(Resource* res){
  res->texture = Screen::load_texture(res->mem->memory, res->mem->size);
}

void ApiManager::update(){
  // If requests are waiting, start request thread
  if(active_res == nullptr && !requests.empty()){

    // Ignore cancelled requests
    while(!requests.empty()){
      active_res = requests.front();
      requests.pop_front();

      if(active_res != nullptr)
        if(active_res->requested)
          break;
    }

    // If all cancelled, set to none
    if(!active_res->requested){
      active_res = nullptr;
      return;
    }

    threadCreate(res_thread, load_res_thread, active_res, 5000, 0x2C, -2);
    threadStart(res_thread);

  } else if (active_res != nullptr){
    // Try and get lock on active_res
    if(mutexTryLock(mutex)){
      // If done, load texture into memory, close thread
      if(active_res->done){
        // If cancelled, don't process
        if(active_res->requested)
          active_res->res_func(active_res);

        // Reset memory struct
        delete active_res->mem;
        active_res->mem = new MemoryStruct();

        // Cleanup thread
        threadWaitForExit(res_thread);
        threadClose(res_thread);

        // Queued for deletion
        if(delete_active){
          delete active_res;
          delete_active = false;
        }
        active_res = nullptr;
      }

      mutexUnlock(mutex);
    }
  }
}

// Cancel all requests queued to be loaded request thread
void ApiManager::cancel_all_requests(){
  for(auto res : requests){
    if(res != nullptr)
      res->requested = 0;
  }
  if(active_res){
    active_res->requested = 0;
  }

  requests.clear();
}

// Threaded image loading
void ApiManager::request_res(Resource* res, void(*res_func)(Resource*)){
  // Active_res is loaded in thread - Don't touch done.
  if(active_res != res)
    res->done = 0;
  res->requested = 1;
  res->res_func = res_func;
  requests.push_back(res);
}

void ApiManager::get_res(MemoryStruct* chunk, std::string url, CURL* curl, int save, std::string path)
{
  FILE* file;
  const char* host = ApiProxy.c_str();

  char *uri = curl_easy_escape(curl, url.c_str(), strlen(url.c_str()));

  char *link = (char*)malloc(strlen(host) + strlen(uri) + 1);
  strcpy(link, host);
  strcat(link, uri);

  // Saving locally, open file
  if(save){
    file = fopen(path.c_str(), "w");
  }

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, link);
    //fprintf(stdout, "Getting Link - %s\n", url.c_str());

    // Check if saving locally or to memory
    if(save){
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    } else {
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) chunk);
    }

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

  	curl_easy_perform(curl);

    curl_free(uri);
    curl_easy_reset(curl);
  }
  if(save)
    fclose(file);
  free(link);
}


json_object* ApiManager::get_res_json(std::string url, CURL* curl){
  std::string readBuffer;
  const char* host = ApiProxy.c_str();

  char *uri = curl_easy_escape(curl, url.c_str(), strlen(url.c_str()));

  char *link = (char*)malloc(strlen(host) + strlen(uri) + 1);
  strcpy(link, host);
  strcat(link, uri);

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, link);
    //fprintf(stdout, "Getting Link - %s\n", url.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

    curl_easy_perform(curl);

    curl_free(uri);
    curl_easy_reset(curl);

    json_object* json = json_tokener_parse(readBuffer.c_str());
    json_object_get(json);
    return json;
  }
  free(link);
  return NULL;
}

json_object* ApiManager::post_api(char* payload, std::string url)
{
  CURL *curl;
  std::string readBuffer;
  const char* host = ApiProxy.c_str();

  curl = curl_easy_init();
  char *uri = curl_easy_escape(curl, url.c_str(), strlen(url.c_str()));

  char *link = (char*)malloc(strlen(host) + strlen(uri) + 1);
  strcpy(link, host);
  strcat(link, uri);

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, link);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
  	curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    // Mark json as in use - caller must put later
    json_object* json = json_tokener_parse(readBuffer.c_str());
    json_object_get(json);
    return json;
  }
  return NULL;
}