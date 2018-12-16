#include "Api_Szuru.hpp"
#include "HSearch.hpp"
#include "Config.hpp"

#define token_template "Authorization: Token %s="

static const std::vector<std::string> json_headers = {"Accept: application/json" , "Content-Type: application/json"};

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf( new char[ size ] ); 
    snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

json_object* get_json_obj(json_object* root, std::string key)
{
  json_object* ret;
  if (json_object_object_get_ex(root, key.c_str(), &ret)){
    return ret;
  }
  return NULL;
}

void Api_Szuru::build_header(std::vector<std::string> headers){
  struct curl_slist *hs=NULL;
  for(auto h : headers){
    hs = curl_slist_append(hs, h.c_str());
  }
  curl_easy_setopt(ApiManager::handle, CURLOPT_HTTPHEADER, hs);
}

bool Api_Szuru::check_token(std::string token){
  std::vector<std::string> headers = json_headers;
  headers.push_back(string_format(token_template, token.c_str()));
  build_header(headers);

  Domain_Szuru* domain = (Domain_Szuru*) HSearch::current_domain();
  std::string url = domain->domain + "api/posts/?offset=1&limit=1&query=";
  json_object* json = ApiManager::get_res_json(url, ApiManager::handle);

  if(json != NULL){
    printf("Responded\n");
    if(get_json_obj(json, "query") != NULL){
      json_object_put(json);
      return true;
    } else {
      printf(json_object_get_string(get_json_obj(json, "title")));
      printf("\n");
      json_object_put(json);
    }
  } else {
    printf("No Response?\n");
  }

  return false;
}

std::string Api_Szuru::update_token(std::string username, std::string password){
  // Headers
  build_header(json_headers);

  // Set up auth
  curl_easy_setopt(ApiManager::handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
  curl_easy_setopt(ApiManager::handle, CURLOPT_USERNAME, username.c_str());
  curl_easy_setopt(ApiManager::handle, CURLOPT_PASSWORD, password.c_str());

  // Create new token
  Domain_Szuru* domain = (Domain_Szuru*) HSearch::current_domain();
  std::string url = domain->domain + "api/user-token/" + username;
  json_object* json = ApiManager::post_api("{}", url, ApiManager::handle);

  if(json == NULL){
    printf("No response\n");
    return "";
  }

  // Error returned
  if(get_json_obj(json, "title") != NULL){
    printf("Error returned %s\n", json_object_to_json_string_ext(json, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
    return "";
  }

  // Return token
  return json_object_get_string(get_json_obj(json, "token"));
}

json_object* Api_Szuru::list_posts(std::string token, int pos, int limit, std::string query){
  Domain_Szuru* domain = (Domain_Szuru*) HSearch::current_domain();
  std::string url = domain->domain + string_format("api/posts/?offset=%d&limit=%d&query=%s", pos, limit, query.c_str());

  // Add auth token and json headers
  std::vector<std::string> headers = json_headers;
  headers.push_back(string_format(token_template, token.c_str()));
  build_header(headers);
  printf("Using Token %s\n", token.c_str());

  // Fetch posts
  return ApiManager::get_res_json(url, ApiManager::handle);
}