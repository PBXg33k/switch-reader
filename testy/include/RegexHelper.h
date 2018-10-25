#ifndef REGEXHELPER_H
#define REGEXHELPER_H

#include <regex>
#include <string>
#include <vector>

class RegexHelper{
  public:
    static std::vector<std::string> search(std::regex re, std::string content);
    static std::smatch search_once(std::regex re, std::string content);
};


#endif
