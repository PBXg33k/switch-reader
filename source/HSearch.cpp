#include "HSearch.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <regex>
#include "RegexHelper.hpp"
#include "Api.hpp"

#define searchURL "https://e-hentai.org/"
#define apiURL "https://g.e-hentai.org/api.php"
#define idRegex "(.+?)(?=\\/)"
#define listXPath "//div[@class='it5']/a"
#define pagesXPath "//p[@class='ip']"

struct ResultsList{
  std::vector<std::string> gids;
  std::vector<std::string> gtkns;
  std::vector<std::string> urls;
};

xmlXPathObjectPtr get_node_set(xmlDocPtr doc, xmlChar *xpath){
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;

    context = xmlXPathNewContext(doc);
    result = xmlXPathEvalExpression(xpath, context);
    return result;
}

json_object* get_json_obj2(json_object* root, const char* key)
{
  json_object* ret;
  if (json_object_object_get_ex(root, key, &ret)){
    return ret;
  }
  return NULL;
}

void HSearch::json_entries(std::vector<std::string> gids, std::vector<std::string> gtkns, std::vector<std::string> urls){
  json_object* json = ApiManager::get_galleries(gids, gtkns);
  json = get_json_obj2(json, "gmetadata");

  // API Call failed, return empty handed
  if(json == NULL){
    return;
  }

  // Push all results to Browser entries
  for(size_t c = 0; c < gids.size(); c++){
    Entry* e = Browser::new_entry(json, c, urls[c]);
    fill_tags(e, json_object_array_get_idx(json, c));
  }

  json_object_put(json);
} 

void HSearch::fill_tags(Entry* entry, json_object* json){
  json_object* holder;
  int numOfTags;
  std::string tag;

  json = get_json_obj2(json, "tags");
  numOfTags = json_object_array_length(json);

  for(int i = 0; i < numOfTags; i++){
    holder = json_object_array_get_idx(json, i);
    tag = json_object_get_string(holder);

    if(tag.find(':') != std::string::npos){
      Tag t( tag.substr(0, tag.find(':')), tag.erase(0, tag.find(':') + 1) );
      if(t.category == "lang" || t.category == "language"){
        if(t.tag != "translated" && t.tag != "speechless"){
          t.tag[0] = std::toupper(t.tag[0]);
          printf("Lang Found : %s\n", t.tag.c_str());
          entry->language = t.tag;
        }
      }
      entry->tags.push_back(t);
    } else {
      Tag t( "misc", tag );
      entry->tags.push_back(t);
    }
  }
}

ResultsList parse_page(MemoryStruct* pageMem, std::string completeURL){
  xmlChar *path;
  xmlChar *url;
  xmlDocPtr doc;
  xmlXPathObjectPtr results;
  xmlNodeSetPtr nodeset;
  std::vector<std::string> idMatches;
  int resultsNum = 0;
  std::regex idPattern(idRegex);
  ResultsList rList;

  doc = htmlReadMemory(pageMem->memory, pageMem->size, completeURL.c_str(), NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

  printf("Loaded as html\n");


  // Find each gallerys URL in page
  path = (xmlChar*) listXPath;
  results = get_node_set(doc, path);
  if(results){
    nodeset = results->nodesetval;
    printf("Matched %d results\n", nodeset->nodeNr);
    // Push each gallery link found onto stack
    for(int c = 0; c < nodeset->nodeNr; c++)
    {
      // Get URL from Href
      url = xmlGetProp(nodeset->nodeTab[c], (xmlChar*) "href");
      rList.urls.push_back((char*)url);
      // Grab GID and GTokens from URL (Used for API call)
      idMatches = RegexHelper::search(idPattern, (char*)url);
      rList.gids.push_back(idMatches[4].substr(1));
      rList.gtkns.push_back(idMatches[5].substr(1));
      xmlFree(url);
    }
  }

  printf("Checking for page results\n");

  path = (xmlChar*) pagesXPath;
  results = get_node_set(doc, path);
  if(results){
    nodeset = results->nodesetval;
    xmlChar* pagesStr = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode, 1);
    std::string content = (char*) pagesStr;
    // Remove commas, take last word/num
    content.erase(std::remove(content.begin(), content.end(), ','), content.end());
    content = content.substr(content.rfind(' ') + 1);

    resultsNum = atoi(content.c_str());
    printf("Results found : %d\n", resultsNum);
    Browser::numOfResults = resultsNum;
  } else {
    Browser::numOfResults = 0;
  }

  // Free up page memory
  xmlFreeDoc(doc);
  xmlCleanupParser();

  return rList;
}

void HSearch::expand_search(std::string completeURL, int page){

  // Add page num
  completeURL += "&page=" + std::to_string(page);


  // Get html page
  MemoryStruct* pageMem = new MemoryStruct();
  ApiManager::get_res(pageMem, completeURL.c_str());

  // Search failed, return empty handed
  if(pageMem->size == 0){
    delete pageMem;
    return;
  }

  ResultsList rList = parse_page(pageMem, completeURL);

  delete pageMem;

  // Nothing found, return emptypageMem handed
  if(rList.gids.empty())
    return;

  json_entries(rList.gids, rList.gtkns, rList.urls);
}

void HSearch::search_keywords(std::string keywords, size_t maxResults, int categories){
  // Build url
  std::string completeURL = searchURL;
  char* safeKeywords;

  // Add selected categories to url using flags
  if (categories & (int) Category::Doujinshi) completeURL.append("?f_doujinshi=1"); else completeURL.append("?f_doujinshi=0");
  if (categories & (int) Category::Manga) completeURL.append("&f_manga=1"); else completeURL.append("&f_manga=0");
  if (categories & (int) Category::ArtistCg) completeURL.append("&f_artistcg=1"); else completeURL.append("&f_artistcg=0");
  if (categories & (int) Category::GameCg) completeURL.append("&f_gamecg=1"); else completeURL.append("&f_gamecg=0");
  if (categories & (int) Category::Western) completeURL.append("&f_western=1"); else completeURL.append("&f_western=0");
  if (categories & (int) Category::NonH) completeURL.append("&f_non-h=1"); else completeURL.append("&f_non-h=0");
  if (categories & (int) Category::ImageSet) completeURL.append("&f_imageset=1"); else completeURL.append("&f_imageset=0");
  if (categories & (int) Category::Cosplay) completeURL.append("&f_cosplay=1"); else completeURL.append("&f_cosplay=0");
  if (categories & (int) Category::AsianPorn) completeURL.append("&f_asianporn=1"); else completeURL.append("&f_asianporn=0");
  if (categories & (int) Category::Misc) completeURL.append("&f_misc=1"); else completeURL.append("&f_misc=0");

  // Convert to URI (Mostly because of search keywords)
  CURL* curl;
  curl = curl_easy_init();
  safeKeywords = curl_easy_escape(curl, keywords.c_str(), strlen(keywords.c_str()));

  ssize_t bufferSize = snprintf(NULL, 0, "&f_search=%s&f_apply=Apply+Filter", safeKeywords);
  char* searchParams = (char*) malloc(bufferSize + 1);
  snprintf(searchParams, bufferSize + 1, "&f_search=%s&f_apply=Apply+Filter", safeKeywords);

  completeURL.append(searchParams);

  printf("Search URL : %s\n", completeURL.c_str());
  Browser::currentUrl = completeURL;
  Browser::loadedPages = 0;

  curl_easy_cleanup(curl);

  // Get html page
  MemoryStruct* pageMem = new MemoryStruct();
  ApiManager::get_res(pageMem, completeURL.c_str());

  // Search failed, return empty handed
  if(pageMem->size == 0){
    delete pageMem;
    return;
  }



  ResultsList rList = parse_page(pageMem, completeURL);

  delete pageMem;

  json_entries(rList.gids, rList.gtkns, rList.urls);
}

std::vector<std::pair<std::string,std::string>> HSearch::get_tags(json_object* json){
  std::vector<std::pair<std::string,std::string>> tagPairs;
  return tagPairs;
}

void HSearch::search_favourites(){
  std::string completeURL = "https://e-hentai.org/favorites.php";

  // Generate s cookie
  MemoryStruct* pageMem = new MemoryStruct();
  ApiManager::get_res(pageMem, completeURL.c_str());
  delete pageMem;

  // The real request
  pageMem = new MemoryStruct();
  ApiManager::get_res(pageMem, completeURL.c_str());

  // Search failed, return empty handed
  if(pageMem->size == 0){
    delete pageMem;
    return;
  }

  printf("Loaded Favourites\n");

  ResultsList rList = parse_page(pageMem, completeURL);

  delete pageMem;

  Browser::currentUrl = completeURL;
  Browser::loadedPages = 0;

  if(rList.gids.size() > 25){
    printf("Making subvectors\n");
    std::vector<std::string> subGids(rList.gids.begin(), rList.gids.begin() + 24);
    std::vector<std::string> subGtkns(rList.gtkns.begin(), rList.gtkns.begin() + 24);
    std::vector<std::string> subUrls(rList.urls.begin(), rList.urls.begin() + 24);
    printf("Subs made\n");

    json_entries(subGids, subGtkns, subUrls);

    std::vector<std::string> subGids2(rList.gids.begin() + 25, rList.gids.end());
    std::vector<std::string> subGtkns2(rList.gtkns.begin() + 25, rList.gtkns.end());
    std::vector<std::string> subUrls2(rList.urls.begin() + 25, rList.urls.end()); 

    json_entries(subGids2, subGtkns2, subUrls2);
  } else {
    json_entries(rList.gids, rList.gtkns, rList.urls);
  }
}