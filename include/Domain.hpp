#pragma once

#include "Api.hpp"
#include "Gallery.hpp"
#include <vector>
#include <libxml/xpath.h>

// Template

struct ResultsList{
  std::vector<std::string> gids;
  std::vector<std::string> gtkns;
  std::vector<std::string> urls;
};

class Domain {
  public:
    virtual void process_gallery_req(Resource* res); /* Process Gallery Page Requests - Page in Resource Meta */
    virtual void search(std::string keywords, std::vector<void*> args = std::vector<void*>()); /* Search and fill Browser */
    virtual void search(std::string keywords, std::string type, std::vector<void*> args); /* Might be used for favourites later? */
    virtual void expand_search(std::string completeURL, int page); /* Adds more results to Browser */
    virtual void search_favourites(); /* Loads the default favourites immediately */
    virtual void prefill_gallery(Entry* e, Gallery* gallery); /* Fills the Gallery's Resource list when loaded, useful if the urls are immediately known */
    virtual int download_gallery(Gallery* gallery, std::string directory); /* Saves all gallery pages. Returns - 1 = Download not supported */
    virtual void load_gallery_urls(size_t page, int* block_size, Gallery* gallery); /* Loads URLs from the next page - Block size is 1 when not set, your job to do so */

    // Shared functions
    static xmlXPathObjectPtr get_node_set(xmlDocPtr doc, xmlChar *xpath);
    static json_object* get_json_obj(json_object* root, std::string key);
};

// Domain list

class Domain_EHentai : public Domain {
  public:
    void process_gallery_req(Resource* res);
    void search(std::string keywords, std::vector<void*> args = std::vector<void*>());
    void expand_search(std::string completeURL, int page);
    void search_favourites();
    int download_gallery(Gallery* gallery, std::string directory);
    void load_gallery_urls(size_t page, int* block_size, Gallery* gallery);
    json_object* get_galleries(std::vector<std::string> gids, std::vector<std::string> gtkns);

    std::string SearchURL = "https://e-hentai.org/";;
    std::string FavouritesURL = "https://e-hentai.org/favorites.php";
    std::string ApiURL = "https://api.e-hentai.org/api.php";

  private:
    bool contains_tag(Entry* e, std::string tag);
    void fill_tags(Entry* entry, json_object* json);
    int json_entries(std::vector<std::string> gids, std::vector<std::string> gtkns, std::vector<std::string> urls);
    ResultsList parse_page(MemoryStruct* pageMem, std::string completeURL);
};

class Domain_NHentai : public Domain {
  public:
    void search(std::string keywords, std::vector<void*> args = std::vector<void*>());
    void expand_search(std::string completeURL, int page);
    void prefill_gallery(Entry* e, Gallery* gallery);

    std::string ApiURL = "https://nhentai.net/api";

  private:
    void parse_page(std::string completeURL, int page);
    std::string build_image(Entry* e, int page, std::string type, bool thumbnail);
};