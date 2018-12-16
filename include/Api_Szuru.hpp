#pragma once

#include "Api.hpp"
#include <string>

class Api_Szuru {
  public:
    static bool check_token(std::string token);
    static std::string update_token(std::string username, std::string password);

    static json_object* list_tags(std::string token, int pos, int limit, std::string query);
    static json_object* get_tag(std::string token, std::string name);

    static json_object* list_posts(std::string token, int pos, int limit, std::string query);
    static json_object* get_post(std::string token, int id);
  private:
    static void build_header(std::vector<std::string> headers);
};