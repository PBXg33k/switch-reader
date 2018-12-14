#pragma once

#include <string>
#include <vector>

class Dialog {
  public:
    static int get_response(std::string message, std::vector<std::string> choices);
};

class Dialog_Confirm : public Dialog {
  public:
    static bool get_bool(std::string message);
};