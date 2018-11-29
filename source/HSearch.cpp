#include "HSearch.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <regex>
#include "RegexHelper.hpp"
#include "Api.hpp"
#include "Config.hpp"

#define idRegex "(.+?)(?=\\/)"
#define listXPath "//div[@class='it5']/a"
#define pagesXPath "//p[@class='ip']"

#define nhThumbnailURL "https://t.nhentai.net"
#define nhImageURL "https://i.nhentai.net"

struct ResultsList{
  std::vector<std::string> gids;
  std::vector<std::string> gtkns;
  std::vector<std::string> urls;
};

std::string HSearch::build_nh_image(Entry* e, int page, std::string type, bool thumbnail){
  std::string imageURL;

  // Thumbnails
  if(thumbnail){
    imageURL = std::string(nhThumbnailURL) + "/galleries/" + std::to_string(e->media_id) + "/thumb";

  // Images
  } else {
    imageURL = std::string(nhImageURL) + "/galleries/" + std::to_string(e->media_id) + "/" + std::to_string(page);
  }

  // Add format
  if(type == "j")
    imageURL += ".jpg";
  else if(type == "p")
    imageURL += ".png";

  return imageURL;
}

xmlXPathObjectPtr get_node_set(xmlDocPtr doc, xmlChar *xpath){
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;

    context = xmlXPathNewContext(doc);
    result = xmlXPathEvalExpression(xpath, context);
    return result;
}

json_object* HSearch::get_json_obj(json_object* root, std::string key)
{
  json_object* ret;
  if (json_object_object_get_ex(root, key.c_str(), &ret)){
    return ret;
  }
  return NULL;
}

bool contains_tag(Entry* e, std::string tag) {
  if(tag.empty())
    return true;

  for(auto t : e->tags){
    if(strcasecmp(tag.c_str(), t.tag.c_str()) == 0)
      return true;
  }

  return false;
}

void parse_nh_page(std::string completeURL, int page){
  json_object* json;
  json_object* array_of_galleries;
  json_object* gallery_info;
  json_object* holder;
  json_object* images;
  std::string copy = completeURL;

  if(completeURL.find('?') != std::string::npos)
    completeURL += "&page=" + std::to_string(page);
  else
    completeURL += "?page=" + std::to_string(page);

  json = ApiManager::get_res_json(completeURL, ApiManager::handle);

  // Failed request
  if(json == NULL){
    return;
  }

  array_of_galleries = HSearch::get_json_obj(json, "result");

  for(int i = 0; i < (int) json_object_array_length(array_of_galleries); i++){
    gallery_info = json_object_array_get_idx(array_of_galleries, i);
    Entry* e = new Entry;

    printf("Adding new gallery\n");

    // Add ID
    holder = HSearch::get_json_obj(gallery_info, "id");
    if(json_object_is_type(holder, json_type_int)){
      e->id = json_object_get_int(holder);
    } else {
      std::string id_str = json_object_get_string(holder);
      e->id = stoi(id_str);
    }

    // Add Media ID
    holder = HSearch::get_json_obj(gallery_info, "media_id");
    std::string id_str = json_object_get_string(holder);
    e->media_id = stoi(id_str);

    // Add title
    holder = HSearch::get_json_obj(gallery_info, "title");
    holder = HSearch::get_json_obj(holder, "english");
    e->title = json_object_get_string(holder);

    printf("Title : %s\n", e->title.c_str());

    // Add pages
    holder = HSearch::get_json_obj(gallery_info, "num_pages");
    e->pages = json_object_get_int(holder);

    printf("Pages : %d\n", e->pages);

    // Add thumbnail
    images = HSearch::get_json_obj(gallery_info, "images");
    holder = HSearch::get_json_obj(images, "thumbnail");
    e->thumb = HSearch::build_nh_image(e, 0, json_object_get_string(HSearch::get_json_obj(holder, "t")), true);

    // Fill irrelevant
    e->category = "";
    e->url = "";
    e->rating = 0;

    printf("Adding tags\n");

    // Add tags - reusing gallery_info for it
    gallery_info = HSearch::get_json_obj(gallery_info, "tags");
    for(int j = 0; j < (int) json_object_array_length(gallery_info); j++){
      holder = json_object_array_get_idx(gallery_info, j);
      std::string type = json_object_get_string(HSearch::get_json_obj(holder, "type"));
      std::string name = json_object_get_string(HSearch::get_json_obj(holder, "name"));

      // Is a language, set as so
      if(type == "language"){
        if(name != "translated" && name != "text cleaned"){
          e->language = name;
          // Make sure language is capitalised - Style is best
          e->language[0] = std::toupper(e->language[0]);
        }
      }
      e->tags.push_back(Tag(type,name));
    }

    Browser::add_entry(e);
  }

  // Set results amount - TODO: Adjust for last page differences
  holder = HSearch::get_json_obj(json, "num_pages");
  
  if(copy.find('?') != std::string::npos)
    copy += "&page=" + std::string(json_object_get_string(holder));
  else
    copy += "?page=" + std::string(json_object_get_string(holder));

  json_object* num_check = ApiManager::get_res_json(copy, ApiManager::handle);
  holder = HSearch::get_json_obj(num_check, "result");


  if(Browser::numOfResults == 0){
    Browser::numOfResults = ((json_object_get_int(HSearch::get_json_obj(json, "num_pages")) - 1) * 25) + json_object_array_length(holder);
    if(Browser::numOfResults < 0)
      Browser::numOfResults = 0;
  }

  json_object_put(num_check);
  json_object_put(json);
}

int HSearch::json_entries(std::vector<std::string> gids, std::vector<std::string> gtkns, std::vector<std::string> urls){
  json_object* json = ApiManager::get_galleries(gids, gtkns);
  int skipped_entries = 0;

  if(json == NULL)
    return 0;

  json = get_json_obj(json, "gmetadata");

  // API Call failed, return empty handed
  if(json == NULL){
    return 0;
  }

  // Push all results to Browser entries
  for(size_t c = 0; c < gids.size(); c++){
    Entry* e = new Entry;
    fill_tags(e, json_object_array_get_idx(json, c));
    if(contains_tag(e, ConfigManager::get_value("lang"))){
      Browser::new_entry(json, e, c, urls[c]);
    } else {
      delete e;
      skipped_entries++;
    }
  }

  json_object_put(json);

  return skipped_entries;
} 

void HSearch::fill_tags(Entry* entry, json_object* json){
  json_object* holder;
  int numOfTags;
  std::string tag;

  json = HSearch::get_json_obj(json, "tags");
  numOfTags = json_object_array_length(json);

  for(int i = 0; i < numOfTags; i++){
    holder = json_object_array_get_idx(json, i);
    tag = json_object_get_string(holder);

    if(tag.find(':') != std::string::npos){
      Tag t( tag.substr(0, tag.find(':')), tag.erase(0, tag.find(':') + 1) );
      if(t.category == "lang" || t.category == "language"){
        if(t.tag != "translated" && t.tag != "speechless"){
          t.tag[0] = std::toupper(t.tag[0]);
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

  if(doc == nullptr)
    return rList;

  // Find each gallerys URL in page
  path = (xmlChar*) listXPath;
  results = get_node_set(doc, path);

  if(xmlXPathNodeSetIsEmpty(results->nodesetval)){
    xmlXPathFreeObject(results);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return rList;
  } else {
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
    xmlXPathFreeObject(results);
  }

  printf("Checking for page results\n");

  path = (xmlChar*) pagesXPath;
  results = get_node_set(doc, path);
  if(xmlXPathNodeSetIsEmpty(results->nodesetval)){
    xmlXPathFreeObject(results);
  } else {
    nodeset = results->nodesetval;
    xmlChar* pagesStr = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode, 1);
    std::string content = (char*) pagesStr;
    // Remove commas, take last word/num
    content.erase(std::remove(content.begin(), content.end(), ','), content.end());
    content = content.substr(content.rfind(' ') + 1);

    resultsNum = atoi(content.c_str());
    printf("Results found : %d\n", resultsNum);
    if(Browser::numOfResults == 0)
      Browser::numOfResults = resultsNum;
  }

  // Free up page memory
  xmlFreeDoc(doc);
  xmlCleanupParser();

  return rList;
}

void HSearch::expand_search(std::string completeURL, int page){
  // If NHentai, skip rest
  if(ConfigManager::get_value("mode") == "NHentai"){
    parse_nh_page(completeURL, page);
    return;
  }

  // Add page num - & if parameters already exist
  if(completeURL.find('?') != std::string::npos)
    completeURL += "&page=" + std::to_string(page);
  else
    completeURL += "?page=" + std::to_string(page);

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

void HSearch::search_eh_keywords(std::string keywords, int categories){
  // Build url
  std::string completeURL;
  if(ConfigManager::get_value("search") == "Public")
    completeURL = SearchURL;
  else
    completeURL = FavouritesURL;

  char* safeKeywords;
  Browser::numOfResults = 0;

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

  // Advanced Searching
  int stars = stoi(ConfigManager::get_value("stars"));
  // Star filter  - Doesn't work below 2 stars
  if(stars > 1){
    completeURL.append("&f_sr=on");
    completeURL.append("&f_srdd=" + std::to_string(stars));
  }

  // Add language filter
  keywords += " " + ConfigManager::get_value("lang");

  // Convert to URI (Mostly because of search keywords)
  CURL* curl;
  curl = curl_easy_init();
  safeKeywords = curl_easy_escape(curl, keywords.c_str(), strlen(keywords.c_str()));

  ssize_t bufferSize = snprintf(NULL, 0, "&f_search=%s&f_apply=Apply+Filter&advsearch=1&f_sname=on&f_stags=on&f_sdesc=on", safeKeywords);
  char* searchParams = (char*) malloc(bufferSize + 1);
  snprintf(searchParams, bufferSize + 1, "&f_search=%s&f_apply=Apply+Filter&advsearch=1&f_sname=on&f_stags=on&f_sdesc=on", safeKeywords);

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

  int skipped_entries;

  if(rList.gids.size() > 25){
    printf("Making subvectors\n");
    std::vector<std::string> subGids(rList.gids.begin(), rList.gids.begin() + 24);
    std::vector<std::string> subGtkns(rList.gtkns.begin(), rList.gtkns.begin() + 24);
    std::vector<std::string> subUrls(rList.urls.begin(), rList.urls.begin() + 24);
    printf("Subs made\n");

    skipped_entries = json_entries(subGids, subGtkns, subUrls);

    std::vector<std::string> subGids2(rList.gids.begin() + 25, rList.gids.end());
    std::vector<std::string> subGtkns2(rList.gtkns.begin() + 25, rList.gtkns.end());
    std::vector<std::string> subUrls2(rList.urls.begin() + 25, rList.urls.end()); 

    skipped_entries += json_entries(subGids2, subGtkns2, subUrls2);
  } else {
    skipped_entries = json_entries(rList.gids, rList.gtkns, rList.urls);
  }

  Browser::numOfResults -= skipped_entries;
}

std::vector<std::pair<std::string,std::string>> HSearch::get_tags(json_object* json){
  std::vector<std::pair<std::string,std::string>> tagPairs;
  return tagPairs;
}

void HSearch::search_eh_favourites(){
  std::string completeURL = FavouritesURL;
  Browser::numOfResults = 0;

  // Generate 's' cookie
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
  int skipped_entries;

  if(rList.gids.size() > 25){
    printf("Making subvectors\n");
    std::vector<std::string> subGids(rList.gids.begin(), rList.gids.begin() + 24);
    std::vector<std::string> subGtkns(rList.gtkns.begin(), rList.gtkns.begin() + 24);
    std::vector<std::string> subUrls(rList.urls.begin(), rList.urls.begin() + 24);
    printf("Subs made\n");

    skipped_entries = json_entries(subGids, subGtkns, subUrls);

    std::vector<std::string> subGids2(rList.gids.begin() + 25, rList.gids.end());
    std::vector<std::string> subGtkns2(rList.gtkns.begin() + 25, rList.gtkns.end());
    std::vector<std::string> subUrls2(rList.urls.begin() + 25, rList.urls.end()); 

    skipped_entries += json_entries(subGids2, subGtkns2, subUrls2);
  } else {
    skipped_entries = json_entries(rList.gids, rList.gtkns, rList.urls);
  }

  Browser::numOfResults -= skipped_entries;
}

void HSearch::search_nh_keywords(std::string keywords){
  std::string completeURL = ApiURL;
  char* safeKeywords;

  if(!keywords.empty()){
    completeURL += "/galleries/search?query=";

    CURL* curl;
    curl = curl_easy_init();
    safeKeywords = curl_easy_escape(curl, keywords.c_str(), strlen(keywords.c_str()));
    completeURL += safeKeywords;
    curl_easy_cleanup(curl);
  } else {
    completeURL+= "/galleries/all";
  }

  Browser::loadedPages = 1;
  Browser::numOfResults = 0;
  Browser::currentUrl = completeURL;

  parse_nh_page(completeURL, 1);

}

void HSearch::search_keywords(std::string keywords, int categories){
  if(ConfigManager::get_value("mode") == "NHentai")
    search_nh_keywords(keywords);
  else
    search_eh_keywords(keywords, categories);
}

json_object* HSearch::fetch_nh_gallery(Entry* e){
  std::string completeURL = ApiURL + "/gallery/" + std::to_string(e->id);
  return ApiManager::get_res_json(completeURL, ApiManager::handle);
}