#include "Api_Lanraragi.hpp"
#include "HSearch.hpp"

typedef std::map<std::string, std::string> Params;

static std::string params_string(Params const &params)
{
  if(params.empty()) return "";
  Params::const_iterator pb= params.cbegin(), pe= params.cend();
  std::string data= pb-> first+ "="+ pb-> second;
  ++ pb; if(pb== pe) return data;
  for(; pb!= pe; ++ pb)
    data+= "&"+ pb-> first+ "="+ pb-> second;
  return data;
}

bool Api_Lanraragi::check_key(std::string key){
  json_object* json = list_posts(key);
   printf("Response : %s", json_object_to_json_string_ext(json, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));

  if(json_object_is_type(json, json_type_array)){
    json_object_put(json);
    return true;
  } else{
    json_object_put(json);
    return false;
  }

}

json_object* Api_Lanraragi::list_posts(std::string key){
  Domain_Lanraragi* domain = (Domain_Lanraragi*) HSearch::current_domain();
  std::string test_url = domain->domain + "api/archivelist?" + params_string({{"key", key.c_str()}});
  return ApiManager::get_res_json(test_url, ApiManager::handle);
}