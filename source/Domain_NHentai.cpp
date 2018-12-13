#include "Domain.hpp"
#include "Browser.hpp"

#define nhThumbnailURL "https://t.nhentai.net"
#define nhImageURL "https://i.nhentai.net"

void Domain_NHentai::search(std::string keywords, std::vector<void*> args){
  std::string completeURL = ApiURL;
  char* safeKeywords;

  if(!keywords.empty()){
    completeURL += "/galleries/search?query=";

    CURL* curl;
    curl = curl_easy_init();
    safeKeywords = curl_easy_escape(curl, keywords.c_str(), strlen(keywords.c_str()));
    completeURL += safeKeywords;
    curl_easy_cleanup(curl);
  } else {
    completeURL+= "/galleries/all";
  }

  Browser::loadedPages = 1;
  Browser::numOfResults = 0;
  Browser::currentUrl = completeURL;

  parse_page(completeURL, 1);

}

void Domain_NHentai::expand_search(std::string completeURL, int page){
  parse_page(completeURL, page);
}

void Domain_NHentai::parse_page(std::string completeURL, int page){
  json_object* json;
  json_object* array_of_galleries;
  json_object* gallery_info;
  json_object* holder;
  json_object* images;
  std::string copy = completeURL;

  if(completeURL.find('?') != std::string::npos)
    completeURL += "&page=" + std::to_string(page);
  else
    completeURL += "?page=" + std::to_string(page);

  json = ApiManager::get_res_json(completeURL, ApiManager::handle);

  // Failed request
  if(json == NULL){
    return;
  }

  array_of_galleries = Domain::get_json_obj(json, "result");

  for(int i = 0; i < (int) json_object_array_length(array_of_galleries); i++){
    gallery_info = json_object_array_get_idx(array_of_galleries, i);
    Entry* e = new Entry;

    printf("Adding new gallery\n");

    // Add ID
    holder = Domain::get_json_obj(gallery_info, "id");
    if(json_object_is_type(holder, json_type_int)){
      e->id = json_object_get_int(holder);
    } else {
      std::string id_str = json_object_get_string(holder);
      e->id = stoi(id_str);
    }

    // Add Media ID
    holder = Domain::get_json_obj(gallery_info, "media_id");
    std::string id_str = json_object_get_string(holder);
    e->media_id = stoi(id_str);

    // Add title
    holder = Domain::get_json_obj(gallery_info, "title");
    holder = Domain::get_json_obj(holder, "english");
    e->title = json_object_get_string(holder);

    printf("Title : %s\n", e->title.c_str());

    // Add pages
    holder = Domain::get_json_obj(gallery_info, "num_pages");
    e->pages = json_object_get_int(holder);

    printf("Pages : %d\n", e->pages);

    // Add thumbnail
    images = Domain::get_json_obj(gallery_info, "images");
    holder = Domain::get_json_obj(images, "thumbnail");
    e->thumb = Domain_NHentai::build_image(e, 0, json_object_get_string(Domain::get_json_obj(holder, "t")), true);

    // Fill irrelevant
    e->category = "";
    e->url = "";
    e->rating = 0;

    printf("Adding tags\n");

    // Add tags - reusing gallery_info for it
    gallery_info = Domain::get_json_obj(gallery_info, "tags");
    for(int j = 0; j < (int) json_object_array_length(gallery_info); j++){
      holder = json_object_array_get_idx(gallery_info, j);
      std::string type = json_object_get_string(Domain::get_json_obj(holder, "type"));
      std::string name = json_object_get_string(Domain::get_json_obj(holder, "name"));

      // Is a language, set as so
      if(type == "language"){
        if(name != "translated" && name != "text cleaned"){
          name[0] = std::toupper(name[0]);
          e->language = name;
        }
      }
      e->tags.insert(std::make_pair(type, name));
    }

    Browser::add_entry(e);
  }

  // Set results amount - TODO: Adjust for last page differences
  holder = Domain::get_json_obj(json, "num_pages");
  
  if(copy.find('?') != std::string::npos)
    copy += "&page=" + std::string(json_object_get_string(holder));
  else
    copy += "?page=" + std::string(json_object_get_string(holder));

  json_object* num_check = ApiManager::get_res_json(copy, ApiManager::handle);
  holder = Domain::get_json_obj(num_check, "result");


  if(Browser::numOfResults == 0){
    Browser::numOfResults = ((json_object_get_int(Domain::get_json_obj(json, "num_pages")) - 1) * 25) + json_object_array_length(holder);
    if(Browser::numOfResults < 0)
      Browser::numOfResults = 0;
  }

  json_object_put(num_check);
  json_object_put(json);
}

std::string Domain_NHentai::build_image(Entry* e, int page, std::string type, bool thumbnail){
  std::string imageURL;

  // Thumbnails
  if(thumbnail){
    imageURL = std::string(nhThumbnailURL) + "/galleries/" + std::to_string(e->media_id) + "/thumb";

  // Images
  } else {
    imageURL = std::string(nhImageURL) + "/galleries/" + std::to_string(e->media_id) + "/" + std::to_string(page);
  }

  // Add format
  if(type == "j")
    imageURL += ".jpg";
  else if(type == "p")
    imageURL += ".png";

  return imageURL;
}

void Domain_NHentai::prefill_gallery(Entry* e, Gallery* gallery){
  json_object* images;
  json_object* img;
  std::string completeURL = ApiURL + "/gallery/" + std::to_string(e->id);

  json_object* json = ApiManager::get_res_json(completeURL, ApiManager::handle);

  images = Domain::get_json_obj(json, "images");
  images = Domain::get_json_obj(images, "pages");
  // If NHentai, build all URLs to start with
  for(int i = 0; i < (int) json_object_array_length(images); i++){
    img = json_object_array_get_idx(images, i);

    Resource* res = new Resource();
    res->populated = 1;
    res->url = build_image(e, i+1, json_object_get_string(Domain::get_json_obj(img, "t")), false);
    gallery->images.push_back(res);
  }
  json_object_put(json);
}
