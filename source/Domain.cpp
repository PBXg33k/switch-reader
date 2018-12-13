#include "Domain.hpp"
#include "Config.hpp"

void Domain::process_gallery_req(Resource* res){}
void Domain::search(std::string keywords, std::vector<void*> args){ Browser::numOfResults = 0; }
void Domain::search(std::string keywords, std::string type, std::vector<void*> args){}
void Domain::expand_search(std::string completeURL, int page){}
void Domain::search_favourites(){}
void Domain::prefill_gallery(Entry* e, Gallery* gallery){}
void Domain::load_gallery_urls(size_t page, int* block_size, Gallery* gallery){}
int Domain::download_gallery(Gallery* gallery, std::string directory){ return 1; }

void Domain::search_touch(){}
void Domain::search_render(){}
HandlerEnum Domain::search_event(int val){ return HandlerEnum::Search; }

xmlXPathObjectPtr Domain::get_node_set(xmlDocPtr doc, xmlChar *xpath){
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;

    context = xmlXPathNewContext(doc);
    result = xmlXPathEvalExpression(xpath, context);
    return result;
}

json_object* Domain::get_json_obj(json_object* root, std::string key)
{
  json_object* ret;
  if (json_object_object_get_ex(root, key.c_str(), &ret)){
    return ret;
  }
  return NULL;
}