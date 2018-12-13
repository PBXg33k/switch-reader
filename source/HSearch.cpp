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

static std::map<std::string, Domain*> domains;

void HSearch::expand_search(std::string completeURL, int page){
  Domain* domain = current_domain();
  domain->expand_search(completeURL, page);
  return;
}

void HSearch::search_keywords(std::string keywords){
  Domain* domain = current_domain();
  domain->search(keywords);
  return;
}

void HSearch::register_domain(std::string name, Domain* domain){
  domains.insert(std::make_pair(name, domain));
}

Domain* HSearch::current_domain(){
  auto iter = domains.find(ConfigManager::get_value("mode"));

  if(iter != domains.end()){
    return iter->second;
  }

  return &default_domain;
}

std::map<std::string, Domain*> HSearch::get_domains(){
  return domains;
}