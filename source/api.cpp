#include "api.h"
#include <iostream>
#include <switch.h>
#include <cstring>

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

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

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void ApiManager::init(){
  socketInitializeDefault();
  curl_global_init(CURL_GLOBAL_ALL);
}

void ApiManager::close(){
  curl_global_cleanup();
  socketExit();
}

json_object* ApiManager::get_gallery(char* g_id, char* g_token){
  char* data = (char*)malloc((strlen(g_id) + strlen(g_token) + 64) * sizeof(char));
  sprintf(data, "{\"method\": \"gdata\",\"gidlist\": [[%s,\"%s\"]],\"namespace\": 1}", g_id, g_token);
  return ApiManager::post_api(data);
}

void ApiManager::url_search(char* keywords)
{
  char* all_tags = "?f_doujinshi=1&f_manga=1&f_artistcg=1&f_gamecg=1&f_western=1&f_non-h=1&f_imageset=1&f_cosplay=1&f_asianporn=1&f_misc=1&f_search=%s&f_apply=Apply+Filter";

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
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
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

MemoryStruct ApiManager::get_res(char* url)
{
  CURL *curl;
  std::string buffer;
  FILE *logfile;
  const char *host = "http://barryis.fun:5000/?url=";

  logfile = fopen("/switch/logget.txt", "wb");

  MemoryStruct chunk;
  chunk.memory = (char*) malloc(1);
  chunk.size = 0;

  curl = curl_easy_init();
  char *uri = curl_easy_escape(curl, url, strlen(url));

  char *link = (char*)malloc(strlen(host) + strlen(uri) + 1);
  strcpy(link, host);
  strcat(link, uri);

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, link);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_STDERR, logfile);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
  	curl_easy_perform(curl);
    fclose(logfile);
    curl_free(uri);
    curl_easy_cleanup(curl);
    return chunk;
  }
  curl_global_cleanup();
  return chunk;
}

json_object* ApiManager::post_api(char* payload)
{
  CURL *curl;
  FILE *logfile;
  std::string readBuffer;

  logfile = fopen("/switch/logpost.txt", "wb");

  curl = curl_easy_init();

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://barryis.fun:5000/");
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_STDERR, logfile);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  	curl_easy_perform(curl);
    fclose(logfile);
    curl_easy_cleanup(curl);
    return json_tokener_parse(readBuffer.c_str());
  }
  curl_global_cleanup();
  const char* empty = "";
  return json_tokener_parse(empty);
}
