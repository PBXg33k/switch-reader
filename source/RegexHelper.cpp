#include "RegexHelper.hpp"

std::vector<std::string> RegexHelper::search(std::regex re, std::string content){
  std::vector<std::string> matches;

  std::sregex_iterator iter(content.begin(), content.end(), re);
  std::sregex_iterator end;

  for(; iter != end; ++iter){
    matches.push_back(iter->str());
  }

  return matches;
}

std::smatch RegexHelper::search_once(std::regex re, std::string content){
  std::smatch match;
  std::regex_search(content, match, re);
  return match;
}
