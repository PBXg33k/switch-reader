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
    delete res;
    res = nullptr;
  } else {
    // Queue to be cleaned up next update - Prevents hangs
    active_res->requested = 0;
    delete_active = true;
  }
}

void load_res_thread(void *args){
  Resource* res = (Resource*) args;

  mutexLock(mutex);
  ApiManager::get_res(res->mem, res->url, ApiManager::thread_handle);
  res->done = 1;
  mutexUnlock(mutex);
}

// Log into site, save cookies
void ApiManager::login(std::string username, std::string password){
  CURL *curl = handle;
  std::string login = "https://forums.e-hentai.org/index.php?act=Login&CODE=01";
  const char* host = ApiProxy.c_str();
  std::string payload;

  // Set login form data
  payload += "{'UserName':'" + username + "', 'PassWord':'" + password + "', 'CookieDate': '1', 'Privacy': '0'}";
  printf("Login Data - %s\n", payload.c_str());

  // Append url to host as a parameter, use as new url
  char *uri = curl_easy_escape(curl, login.c_str(), strlen(login.c_str()));
  char *link = (char*)malloc(strlen(host) + strlen(uri) + 1);
  strcpy(link, host);
  strcat(link, uri);

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, link);

    // Cookies
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

  	curl_easy_perform(curl);
    curl_easy_reset(curl);
  }

  // Cleanup
  free(link);

}

int ApiManager::download_gallery(Entry* entry, float* percent){
  struct stat info;
  std::string path = "/switch/Reader/" + std::to_string(entry->id);

  // Create gallery directory
  stat(path.c_str(), &info);
  if(!(info.st_mode & S_IFDIR)){
    printf("Creating gallery directory\n");
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }

  // Save info
  ConfigManager::save_entry_info(entry);

  // Save thumbnail
  ApiManager::get_res(nullptr, entry->thumb, ApiManager::handle, 1, path + "/thumb.jpg");

  // Save pages
  GalleryBrowser::load_gallery(entry);
  return GalleryBrowser::save_all_pages(path);
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
        // If cancelled, don't load texture
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

json_object* ApiManager::get_galleries(std::vector<std::string> gids, std::vector<std::string> gtkns){
  char* temp = (char *) "[%s,\"%s\"]";
  std::string gallery_list;
  size_t size;
  char* buffer;

  printf("Making string\n");

  if(gids.size() == 0)
    return NULL;

  for(size_t c = 0; c < gids.size(); c++){
    size = snprintf(NULL, 0, temp, gids[c].c_str(), gtkns[c].c_str());
    buffer = (char*) malloc(size + 1);
    snprintf(buffer, size + 1, temp, gids[c].c_str(), gtkns[c].c_str());
    gallery_list.append(buffer);
    gallery_list.append(",");
    free(buffer);
  }

  gallery_list.resize(gallery_list.size() - 1);

  char* data = (char*)malloc((strlen(gallery_list.c_str()) + 64) * sizeof(char));
  sprintf(data, "{\"method\": \"gdata\",\"gidlist\": [%s],\"namespace\": 1}", gallery_list.c_str());
  printf("%s\n",data);
  json_object* json = ApiManager::post_api(data, ApiURL);
  free(data);
  return json;
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
    printf("Getting Link - %s\n", link);

    // Cookies
    //curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/switch/Reader/cookies");

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
    printf("Getting Link - %s\n", link);

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
    //curl_easy_setopt(curl, CURLOPT_URL, "https://api.e-hentai.org/api.php");
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    // Cookies
    //curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/switch/Reader/cookies");

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