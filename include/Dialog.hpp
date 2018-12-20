#pragma once

#include <string>
#include <vector>

class Dialog {
  public:
    static int get_response(std::string message, std::vector<std::string> choices);
    static int get_number(std::string message, int rot, int max, int start=0);
    static bool get_bool(std::string message);
};