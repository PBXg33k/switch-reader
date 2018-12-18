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
    virtual void search(std::string keywords); /* REQUIRED - Search and fill Browser */
    virtual void search(std::string keywords, std::string type); /* Might be used for favourites later? */
    virtual void expand_search(std::string completeURL, int page); /* REQUIRED - Adds more results to Browser */
    virtual void search_favourites(); /* Loads the default favourites immediately */
    virtual void prefill_gallery(Entry* e, Gallery* gallery); /* Fills the Gallery's Resource list when loaded, useful if the urls are immediately known */
    virtual int download_gallery(Gallery* gallery); /* Saves all gallery pages. Returns - 1 = Download not supported, Other = Failure */
    virtual void load_gallery_urls(size_t page, int* block_size, Gallery* gallery); /* Loads URLs from the next page - Block size is 1 when not set, your job to do so */
    virtual void login(std::string username, std::string password); /* Log into domain, will save cookies - May collide with others when using Proxy */
    virtual std::string get_username(); /* Fetch username of logged in user (none otherwise) */

    // Search page extension
    virtual void search_touch();
    virtual void search_render();
    virtual HandlerEnum search_event(int val);

    // Browser page extension
    virtual void browser_touch();
    virtual void browser_render();
    virtual HandlerEnum browser_event(int val);

    // Settings page extension
    virtual void settings_touch();
    virtual void settings_render();
    virtual HandlerEnum settings_event(int val);

    // Gallery Preview page extension
    virtual void preview_touch();
    virtual void preview_render();
    virtual HandlerEnum preview_event(int val);

    // Shared functions
    static xmlXPathObjectPtr get_node_set(xmlDocPtr doc, xmlChar *xpath);
    static json_object* get_json_obj(json_object* root, std::string key);
};

// Domain list

class Domain_EHentai : public Domain {
  public:
    void process_gallery_req(Resource* res);
    void search(std::string keywords);
    void expand_search(std::string completeURL, int page);
    void search_favourites();
    int download_gallery(Gallery* gallery);
    void load_gallery_urls(size_t page, int* block_size, Gallery* gallery);
    json_object* get_galleries(std::vector<std::string> gids, std::vector<std::string> gtkns);
    void login(std::string username, std::string password);
    std::string get_username();

    void preview_touch();
    void preview_render();
    HandlerEnum preview_event(int val);

    void search_touch();
    void search_render();
    HandlerEnum search_event(int val);

    std::string SearchURL = "https://e-hentai.org/";
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
    void search(std::string keywords);
    void expand_search(std::string completeURL, int page);
    void prefill_gallery(Entry* e, Gallery* gallery);
    int download_gallery(Gallery * gallery);

    std::string ApiURL = "https://nhentai.net/api";

  private:
    void parse_page(std::string completeURL, int page);
    std::string build_image(Entry* e, int page, std::string type, bool thumbnail);
    bool contains_tag(Entry* e, std::string tag);
};

class Domain_Local : public Domain {
  public:
    void search(std::string keywords);
    void prefill_gallery(Entry* e, Gallery* gallery);
  private:
    std::vector<std::string> get_directories(const std::string& s);
};

class Domain_Szuru : public Domain {
  public:
    std::string domain;
    std::string name;
    std::string username;

    void search(std::string keywords);
    void login(std::string username, std::string password);
    void prefill_gallery(Entry* e, Gallery* gallery);
    void load_gallery_urls(size_t page, int* block_size, Gallery* gallery);
    int download_gallery(Gallery* gallery);
    std::string get_username();

    void browser_touch();
    void browser_render();
    HandlerEnum browser_event(int val);

    void preview_touch();
    void preview_render();
    HandlerEnum preview_event(int val);

    void settings_touch();
    void settings_render();
    HandlerEnum settings_event(int val);
    

    Domain_Szuru(std::string d, std::string n){
      username = "Not Logged In";
      search_entry = nullptr;
      domain = d;
      name = n;
    }
  private:
    std::multimap<std::string, std::string> query_to_tags(std::string query);
    Entry* search_entry;
    void empty_search();
    std::string token;
    bool token_valid();
};