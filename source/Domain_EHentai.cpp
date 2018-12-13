#include "Domain.hpp"
#include "RegexHelper.hpp"
#include "Browser.hpp"
#include "Config.hpp"

#include <libxml/HTMLparser.h>

#define idRegex "(.+?)(?=\\/)"
#define listXPath "//div[@class='it5']/a"
#define pagesXPath "//p[@class='ip']"
#define urlsXPath "//a[contains(@href, 'hentai.org/s/')]"

#define ehSearchUrl "https://e-hentai.org/"
#define ehFavouritesURL "https://e-hentai.org/favorites.php"
#define ehApiURL "https://api.e-hentai.org/api.php"

#define exSearchUrl "https://exhentai.org/"
#define exFavouritesURL "https://exhentai.org/favorites.php"
#define exApiURL "https://api.e-hentai.org/api.php"

#define imageXPath "//img[@id='img']"

void Domain_EHentai::process_gallery_req(Resource* res){

  // Already filled, maybe by older request?
  if(res->populated){
    ApiManager::request_res(res);
    return;
  }

  printf("Populating resource\n");
  xmlChar *path;
  xmlChar *keyword = NULL;
  xmlXPathObjectPtr result;
  xmlDocPtr doc;
  xmlNodeSetPtr nodeset;

  // If page failed to load, return failure image
  if(res->mem->size == 0){
    res->texture = Screen::load_stored_texture(0);
  }

  doc = htmlReadMemory(res->mem->memory, res->mem->size, "base", NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

  if(doc == nullptr){
    xmlCleanupParser();
    return;
  }

  // Get node list matching XPath for direct image url
  path = (xmlChar*) imageXPath;
  result = get_node_set(doc, path);
  if(result){
    nodeset = result->nodesetval;
    keyword = xmlGetProp(nodeset->nodeTab[0], (xmlChar*) "src");
  }

  // Load URL into resource, then request
  if(keyword){
    printf("Requesting page %d - %s\n", res->meta, (char*) keyword);
    res->url = (char*)keyword;
    res->populated = 1;
    ApiManager::request_res(res);
    printf("Sent for resource\n");
  }

  xmlFree(keyword);
  xmlFreeDoc(doc);
  xmlCleanupParser();
}

void Domain_EHentai::fill_tags(Entry* entry, json_object* json){
  json_object* holder;
  int numOfTags;
  std::string tag;

  json = Domain::get_json_obj(json, "tags");
  numOfTags = json_object_array_length(json);

  for(int i = 0; i < numOfTags; i++){
    holder = json_object_array_get_idx(json, i);
    tag = json_object_get_string(holder);

    if(tag.find(':') != std::string::npos){
      std::string type = tag.substr(0, tag.find(':'));
      std::string name = tag.erase(0, tag.find(':') + 1);

      if(type == "lang" || type == "language"){
        if(name != "translated" && name != "speechless"){
          name[0] = std::toupper(name[0]);
          entry->language = name;
        }
      }
      entry->tags.insert(std::make_pair(type, name));
    } else {
      entry->tags.insert(std::make_pair("misc", tag));
    }
  }
}

bool Domain_EHentai::contains_tag(Entry* e, std::string tag) {
  if(tag.empty())
    return true;

  for(auto t : e->tags){
    if(strcasecmp(tag.c_str(), t.second.c_str()) == 0)
      return true;
  }

  return false;
}

int Domain_EHentai::json_entries(std::vector<std::string> gids, std::vector<std::string> gtkns, std::vector<std::string> urls){
  json_object* json = get_galleries(gids, gtkns);
  int skipped_entries = 0;

  if(json == NULL)
    return 0;

  json = Domain::get_json_obj(json, "gmetadata");

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

ResultsList Domain_EHentai::parse_page(MemoryStruct* pageMem, std::string completeURL){
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
  results = Domain::get_node_set(doc, path);

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
  results = Domain::get_node_set(doc, path);
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

void Domain_EHentai::search(std::string keywords, std::vector<void*> args){
  printf("Called 10!\n");

  // Build url

  std::string completeURL;
  if(ConfigManager::get_value("search") == "Public")
    completeURL = SearchURL;
  else
    completeURL = FavouritesURL;

  char* safeKeywords;
  Browser::numOfResults = 0;

  int categories = 0;
  if(args.size() > 0)
    categories = *((int*) args[0]);

  printf("Categories : %d\n", categories);  

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

void Domain_EHentai::expand_search(std::string completeURL, int page){
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

void Domain_EHentai::search_favourites(){
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

int Domain_EHentai::download_gallery(Gallery* gallery, std::string directory){
  for(int page = 0; page < (int) gallery->images.size(); page++){
    // Update progress
    int progress = ((float) page / (float) gallery->total_pages) * ((screen_width / 2) - 10);

    Screen::clear(ThemeBG);
    std::string to_print = "Downloading Gallery - Page " + std::to_string(page + 1) + " of " + std::to_string(gallery->images.size());
    Screen::draw_text_centered(to_print, 0, (screen_height / 2) - 120, screen_width, 100, ThemeText, Screen::header);
    Screen::draw_rect(screen_width / 4, screen_height / 2, screen_width / 2, 150, ThemePanelDark);
    Screen::draw_rect(screen_width / 4 + 5, (screen_height / 2) + 5, progress, 140, ThemePanelLight);
    Screen::render();

    xmlChar *path;
    xmlChar *keyword = NULL;
    xmlXPathObjectPtr result;
    xmlDocPtr doc;
    xmlNodeSetPtr nodeset;

    // Get html page
    MemoryStruct* pageMem = new MemoryStruct();
    ApiManager::get_res(pageMem, gallery->images[page]->url.c_str());

    // If page failed to load, use failure image
    if(pageMem->size == 0){
      delete pageMem;
      gallery->images[page]->texture = Screen::load_stored_texture(0);
    }

    doc = htmlReadMemory(pageMem->memory, pageMem->size, gallery->index.c_str(), NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

    // Get node list matching XPath for direct image url
    path = (xmlChar*) imageXPath;
    result = get_node_set(doc, path);
    if(result){
      nodeset = result->nodesetval;
      keyword = xmlGetProp(nodeset->nodeTab[0], (xmlChar*) "src");
    }

    // Save image
    if(keyword){
      printf("Saving page %d\n", page);
      ApiManager::get_res(NULL, (char*) keyword, ApiManager::handle, 1, directory + "/page" + std::to_string(page) + ".jpg");
    }

    xmlFree(keyword);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    delete pageMem;
  }

  return 0;
}

void Domain_EHentai::load_gallery_urls(size_t page, int* block_size, Gallery* gallery){
  xmlChar *path = (xmlChar*) urlsXPath;
  xmlChar *keyword;

  int i;

  // Load page into memory
  std::string indexCopy = gallery->index;
  indexCopy.append("?p=");
  indexCopy.append(std::to_string(page));
  printf("Fetching %s\n", indexCopy.c_str());
  MemoryStruct* index = new MemoryStruct();
  ApiManager::get_res(index, indexCopy);

  if(index->size == 0){
    delete index;
    return;
  }

  // Load to xml
  xmlDocPtr doc = htmlReadMemory(index->memory, index->size, gallery->index.c_str(), NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

  if(doc == nullptr){
    xmlCleanupParser();
    return;
  }

  // Get node list matching XPath
  xmlXPathObjectPtr result = Domain::get_node_set(doc, path);

  // Iterate through results
  if(result) {
    xmlNodeSetPtr nodeset = result->nodesetval;
    printf("Found %d entries\n", nodeset->nodeNr);
    if(*block_size == 1)
      *block_size = nodeset->nodeNr;

    for (i=0; i < nodeset->nodeNr; i++) {
			keyword = xmlGetProp(nodeset->nodeTab[i], (xmlChar*) "href");
  		printf("Page %d: %s\n", i, keyword);
      Resource* res = new Resource();
      res->url = (char*) keyword;
      gallery->images.push_back(res);
  		xmlFree(keyword);
		}
  }

  printf("Loaded URL block\n");

  // Cleanup
  xmlFreeDoc(doc);
  xmlCleanupParser();
  delete index;
}

json_object* Domain_EHentai::get_galleries(std::vector<std::string> gids, std::vector<std::string> gtkns){
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