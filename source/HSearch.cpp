#include "HSearch.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <regex>
#include "RegexHelper.hpp"


#define searchURL "https://e-hentai.org/"
#define apiURL "https://g.e-hentai.org/api.php"
#define idRegex "(.+?)(?=\\/)"
#define listXPath "//div[@class='it5']/a"

xmlXPathObjectPtr get_node_set(xmlDocPtr doc, xmlChar *xpath){
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;

    context = xmlXPathNewContext(doc);
    result = xmlXPathEvalExpression(xpath, context);
    return result;
}



void HSearch::search_keywords(std::string keywords, size_t maxResults, int categories){

  xmlChar *path;
  xmlChar *url;
  xmlDocPtr doc;
  xmlXPathObjectPtr results;
  xmlNodeSetPtr nodeset;
  std::vector<std::string> idMatches;

  std::vector<std::string> gids;
  std::vector<std::string> gtkns;
  std::vector<std::string> urls;

  std::regex idPattern(idRegex);

  // Build url
  std::string completeURL = searchURL;
  char* safeKeywords;

  // TODO: Implement Tags
  //completeURL.append("?f_doujinshi=1&f_manga=1&f_artistcg=1&f_gamecg=1&f_western=1&f_non-h=1&f_imageset=1&f_cosplay=1&f_asianporn=1&f_misc=1");

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

  // Format keywords for URL
  CURL* curl;
  curl = curl_easy_init();
  safeKeywords = curl_easy_escape(curl, keywords.c_str(), strlen(keywords.c_str()));

  ssize_t bufferSize = snprintf(NULL, 0, "&f_search=%s&f_apply=Apply+Filter", safeKeywords);
  char* searchParams = (char*) malloc(bufferSize + 1);
  snprintf(searchParams, bufferSize + 1, "&f_search=%s&f_apply=Apply+Filter", safeKeywords);

  completeURL.append(searchParams);

  printf("Search URL : %s\n", completeURL.c_str());

  curl_easy_cleanup(curl);

  // XML //

  // Get html page
  MemoryStruct* pageMem = new MemoryStruct();
  ApiManager::get_res(pageMem, completeURL.c_str());

  // Search failed, return empty handed
  if(pageMem->size == 0){
    delete pageMem;
    return;
  }

  doc = htmlReadMemory(pageMem->memory, pageMem->size, completeURL.c_str(), NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

  // Get node list matching XPath for table rows, print first rows content
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
      urls.push_back((char*)url);
      // Grab GID and GTokens from URL (Used for API call)
      idMatches = RegexHelper::search(idPattern, (char*)url);
      gids.push_back(idMatches[4].substr(1));
      gtkns.push_back(idMatches[5].substr(1));
      xmlFree(url);
    }
  }

  // Free up page memory
  xmlFreeDoc(doc);
  xmlCleanupParser();
  delete pageMem;

  // Nothing found, return empty handed
  if(gids.empty())
    return;

  json_object* json = ApiManager::get_galleries(gids, gtkns);

  // API Call failed, return empty handed
  if(json == NULL){
    return;
  }

  for(size_t c = 0; c < gids.size(); c++){
    Browser::new_entry(json, c, urls[c]);
  }

  // Unmark as in use IMPORTANT
  json_object_put(json);
}
