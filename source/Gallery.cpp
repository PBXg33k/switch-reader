#include "Gallery.hpp"
#include "TouchManager.hpp"
#include "Api.hpp"
#include "Ui.hpp"
#include "Browser.hpp"
#include <cstdlib>


#define pagesXPath "//a[starts-with(@href, 'https://e-hentai.org/s/')]"
#define imageXPath "//img[@id='img']"

Gallery* GalleryBrowser::active_gallery;
int GalleryBrowser::cur_page = 0;
const int GalleryBrowser::buffer_size = 2; // 1 - 1 Page, 2 - 3 pages, 3 - 5 pages...
std::vector<Resource*> GalleryBrowser::img_buffer;

void GalleryBrowser::close(){
  if(active_gallery){
    delete active_gallery;
  }

  for(auto res : img_buffer){
    Screen::cleanup_texture(res->texture);
  }

  img_buffer.clear();
}

// Load gallery
void GalleryBrowser::load_gallery(Entry* entry){
  printf("-- LOADING GALLERY --\nTitle: %s\nURL: %s\nPages:%d\n", entry->title.c_str(), entry->url.c_str(), entry->pages);
  active_gallery = new Gallery();
  active_gallery->title = entry->title;
  active_gallery->index = entry->url;
  active_gallery->total_pages = entry->pages;

  // Null all buffer pages
  for(int i = 0; i < active_gallery->total_pages; i++){
    Resource* res = new Resource;
    img_buffer.push_back(res);
  }

  GalleryBrowser::load_urls(0);

  // Populate buffer

  // Loads last first, TODO CHANGE API HANDLING FIRST
  for(int i = buffer_size - 1; i >= 0; i--){
    GalleryBrowser::load_page(i);
  }

  cur_page = 0;
}


// Set up touch controls
void GalleryBrowser::set_touch(){
  TouchManager::clear();

  // Forwards
  TouchManager::add_bounds(screen_width-90, (screen_height/2) - 40, 80, 80, 1);

  // Back
  TouchManager::add_bounds(10, (screen_height/2) - 40, 80, 80, 2);

  // Browser
  TouchManager::add_bounds(screen_width - 75, 0, 75, 75, 101);
}

void GalleryBrowser::load_page(int page){

  // Load more URLs if needed - 40 on each page
  if(page >= (int) active_gallery->pages.size()){
    int block = (active_gallery->pages.size()/40);
    printf("Loading page block %d\n", block);
    load_urls(block);
  }

  xmlChar *path;
  xmlChar *keyword = NULL;
  xmlXPathObjectPtr result;
  xmlDocPtr doc;
  xmlNodeSetPtr nodeset;

  printf("Loading page %d\n", page);

  // Get html page
  MemoryStruct* pageMem = new MemoryStruct();
  ApiManager::get_res(pageMem, active_gallery->pages[page].c_str());

  // If page failed to load, return failure image
  if(pageMem->size == 0){
    delete pageMem;
    img_buffer[page]->texture = Screen::load_stored_texture(0);
  }

  doc = htmlReadMemory(pageMem->memory, pageMem->size, active_gallery->index.c_str(), NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

  // Get node list matching XPath for direct image url
  path = (xmlChar*) imageXPath;
  result = get_node_set(doc, path);
  if(result){
    nodeset = result->nodesetval;
    keyword = xmlGetProp(nodeset->nodeTab[0], (xmlChar*) "src");
  }

  // Request image to be loaded
  if(keyword){
    img_buffer[page]->url = (char*)keyword;
    img_buffer[page]->requested = 1;
    printf("Requesting %s\n", img_buffer[page]->url.c_str());
    ApiManager::request_res(img_buffer[page]);
  }

  xmlFree(keyword);
  xmlFreeDoc(doc);
  xmlCleanupParser();
  delete pageMem;
}

Handler GalleryBrowser::on_event(int val){
  // Next page
  if(val == 1 && cur_page < (active_gallery->total_pages - 1)){
    int to_remove = cur_page - buffer_size;
    int to_add = cur_page + buffer_size;
    cur_page++;

    // Unload oldest page
    if(to_remove >= 0){
      img_buffer[to_remove]->requested = 0;
      img_buffer[to_remove]->done = 0;
      Screen::cleanup_texture(img_buffer[to_remove]->texture);
    }

    // Load next page
    if(to_add < active_gallery->total_pages - 1){
      GalleryBrowser::load_page(to_add);
    }
  }

  // Previous page
  if(val == 2 && cur_page > 0){
    int to_remove = cur_page + buffer_size;
    int to_add = cur_page - buffer_size;
    cur_page--;

    // Unload oldest page
    if(to_remove >= 0){
      img_buffer[to_remove]->requested = 0;
      img_buffer[to_remove]->done = 0;
      Screen::cleanup_texture(img_buffer[to_remove]->texture);
    }

    // Load next page
    if(to_add < active_gallery->total_pages - 1){
      GalleryBrowser::load_page(to_add);
    }
  }

  // Back to browser
  if(val == 101){
    close();
    Browser::set_touch();
    return Handler::Browser;
  }
  return Handler::Gallery;
}


xmlXPathObjectPtr GalleryBrowser::get_node_set(xmlDocPtr doc, xmlChar *xpath){
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;

    // Get doc context to do XPath on
    context = xmlXPathNewContext(doc);
    result = xmlXPathEvalExpression(xpath, context);
    return result;
}

// Buffer page urls on numbered index page (max 40 per index page)
void GalleryBrowser::load_urls(size_t page){

  xmlChar *path = (xmlChar*) pagesXPath;
  xmlChar *keyword;

  int i;

  // Load page into memory
  std::string indexCopy = active_gallery->index;
  indexCopy.append("?p=");
  indexCopy.append(std::to_string(page));
  printf("Fetching %s\n", indexCopy.c_str());
  MemoryStruct* index = new MemoryStruct();
  printf("Made struct\n");
  ApiManager::get_res(index, indexCopy);

  printf("Loaded page\n");

  // Load to xml
  xmlDocPtr doc = htmlReadMemory(index->memory, index->size, active_gallery->index.c_str(), NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

  // Get node list matching XPath
  xmlXPathObjectPtr result = get_node_set(doc, path);

  // Iterate through results
  if(result) {
    xmlNodeSetPtr nodeset = result->nodesetval;
    printf("Found %d entries\n", nodeset->nodeNr);
    for (i=0; i < nodeset->nodeNr; i++) {
			keyword = xmlGetProp(nodeset->nodeTab[i], (xmlChar*) "href");
  		printf("Page %d: %s\n", i, keyword);
      active_gallery->pages.push_back((char *)keyword);
  		xmlFree(keyword);
		}
  }

  // Cleanup
  xmlFreeDoc(doc);
  xmlCleanupParser();
  delete index;
}

void GalleryBrowser::render(){
  Screen::clear(ThemeBG);
  // Image (If loaded)
  if(img_buffer[cur_page]->texture)
    Screen::draw_adjusted_mem(img_buffer[cur_page]->texture, 100, 0, screen_width - 100, screen_height);
  // Right side button (next and kill)
  Screen::draw_button(screen_width-90, (screen_height/2) - 40, 80, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);
  // Left side button (prev)
  Screen::draw_button(10, (screen_height/2) - 40, 80, 80, ThemeButton, ThemeButtonBorder, 4);
}
