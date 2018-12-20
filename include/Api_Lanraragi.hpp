#pragma once

#include <string>
#include <json-c/json.h>

class Api_Lanraragi {
  public:
    static bool check_key(std::string key);

    static json_object* list_posts(std::string key);
};