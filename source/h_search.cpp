#include "h_search.h"

#include <regex>
#include <iostream>

#define searchURL "https://e-hentai.org/"
#define apiURL "https://g.e-hentai.org/api.php"
#define listRegex "<tr class=\"gtr[01]\">.+?<\\/tr>"
#define imgRegex "(<img src=\")(.+?)(?=\")"
#define textRegex "(?<=alt=\")(.+?)(?=\")"
#define linkRegex "(\"it5\"><a href=)(\".+?)(?=\" onmouse)"
#define idRegex "(.+?)(?=\\/)"

std::vector<std::string> search(std::regex re, std::string content){
  std::vector<std::string> matches;

  std::sregex_iterator iter(content.begin(), content.end(), re);
  std::sregex_iterator end;

  for(; iter != end; ++iter){
    matches.push_back(iter->str());
  }

  return matches;
}

std::smatch search_once(std::regex re, std::string content){
  std::smatch match;
  std::regex_search(content, match, re);
  return match;
}

std::vector<Entry> HSearch::search_keywords(char* keywords, int maxResults){

  std::vector<Entry> result;

  // Build url
  std::string completeURL = searchURL;
  char* safeKeywords;

  // TODO: Implement Tags
  //completeURL.append("?f_doujinshi=1&f_manga=1&f_artistcg=1&f_gamecg=1&f_western=1&f_non-h=1&f_imageset=1&f_cosplay=1&f_asianporn=1&f_misc=1");
  completeURL.append("?f_non-h=1");

  // Format keywords for URL
  CURL* curl;
  curl = curl_easy_init();
  safeKeywords = curl_easy_escape(curl, keywords, strlen(keywords));

  ssize_t bufferSize = snprintf(NULL, 0, "&f_search=%s&f_apply=Apply+Filter", safeKeywords);
  char* searchParams = (char*) malloc(bufferSize + 1);
  snprintf(searchParams, bufferSize + 1, "&f_search=%s&f_apply=Apply+Filter", safeKeywords);

  completeURL.append(searchParams);

  // TODO : Remove, debug
  printf("%s\n", completeURL.c_str());

  // Fetch webpage as string
  MemoryStruct searchPage = ApiManager::get_res(completeURL.c_str());
  std::string webPage(searchPage.memory);

  //printf(webPage.c_str());

  // Setup regexes
  std::regex listPattern(listRegex);
  std::regex imgPattern(imgRegex);
  //std::regex textPattern(textRegex);
  std::regex linkPattern(linkRegex);
  std::regex idPattern(idRegex);

  // TODO : Remove, it works, but I don't wanna
  std::string test_string = "<img src=\"dicks.jpg\"></img>";
  std::smatch res = search_once(imgPattern, test_string);
  printf("Match %s\n", res[0].str().c_str());
  printf("True Match %s\n", res[2].str().c_str());

  // Find gallery list
  std::vector<std::string> listMatches = search(listPattern, webPage);
  printf("Found %d Entries\n", listMatches.size());
  for(int c=0; (c < listMatches.size()) && (c <= maxResults); c++){
    // Get url from gallery
    std::smatch linkMatch = search_once(linkPattern, listMatches[c].c_str());
    printf("Link: %s\n", linkMatch[2].str().c_str());

    // Get ID and Token from URL
    std::vector<std::string> idMatches = search(idPattern, linkMatch[2].str().c_str());
    printf("ID : %s Token %s\n", idMatches[4].substr(1).c_str(), idMatches[5].substr(1).c_str());

    // Save to entry and push onto stack
    json_object* json = ApiManager::get_gallery(idMatches[4].substr(1).c_str(), idMatches[5].substr(1).c_str());
    struct Entry entry = Browser::new_entry(json, 0);
    entry.url = linkMatch[2].str().c_str();
    result.push_back(entry);

    printf("Title : %s\nCategory : %s\nThumb : %s\n", entry.title, entry.category, entry.thumb);
  }

  return result;
}
