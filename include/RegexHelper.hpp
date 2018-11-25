#pragma once

#include <regex>
#include <string>
#include <vector>

namespace RegexHelper{
  std::vector<std::string> search(std::regex re, std::string content);
  std::smatch search_once(std::regex re, std::string content);
};
