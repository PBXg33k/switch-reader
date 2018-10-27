#include "Gallery.hpp"
#include "TouchManager.hpp"
#include "Api.hpp"
#include "Ui.hpp"
#include "Browser.hpp"
#include <cstdlib>


#define pagesXPath "//a[starts-with(@href, 'https://e-hentai.org/s/')]"
#define imageXPath "//img[@id='img']"

Gallery* GalleryBrowser::active_gallery;
SDL_Texture* GalleryBrowser::active_image;
SDL_Texture* GalleryBrowser::prev_image;
SDL_Texture* GalleryBrowser::next_image;
int GalleryBrowser::cur_page = 0;

void GalleryBrowser::close(){
  if(active_gallery){
    delete active_gallery;
  }

  if(!prev_image){
    Screen::cleanup_texture(prev_image);
    prev_image = NULL;
  }
  if(!active_image){
    Screen::cleanup_texture(active_image);
    active_image = NULL;
  }
  if(!next_image){
    Screen::cleanup_texture(next_image);
    next_image = NULL;
  }
}

// Load gallery
void GalleryBrowser::load_gallery(Entry* entry){
  printf("-- LOADING GALLERY --\nTitle: %s\nURL: %s\nPages:%d\n", entry->title.c_str(), entry->url.c_str(), entry->pages);
  active_gallery = new Gallery();
  active_gallery->title = entry->title;
  active_gallery->index = entry->url;
  active_gallery->total_pages = entry->pages;

  prev_image = NULL;
  active_image = NULL;
  next_image = NULL;

  GalleryBrowser::load_urls(0);
  active_image = GalleryBrowser::load_page(0);
  next_image = GalleryBrowser::load_page(1);
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

SDL_Texture* GalleryBrowser::load_page(size_t page){

  // Load more URLs if needed - 40 on each page
  if(page >= active_gallery->pages.size()){
    int block = (active_gallery->pages.size()/40);
    printf("Loading block %d\n", block);
    load_urls(block);
  }

  xmlChar *path;
  xmlChar *keyword = NULL;
  xmlXPathObjectPtr result;
  xmlDocPtr doc;
  xmlNodeSetPtr nodeset;

  printf("Loading page %zd\n", page);

  // Get html page
  MemoryStruct* pageMem = new MemoryStruct();
  ApiManager::get_res(pageMem, active_gallery->pages[page].c_str());

  // If page failed to load, return failure image
  if(pageMem->size == 0){
    delete pageMem;
    return Screen::load_stored_texture(0);
  }

  doc = htmlReadMemory(pageMem->memory, pageMem->size, active_gallery->index.c_str(), NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

  // Get node list matching XPath for image
  path = (xmlChar*) imageXPath;
  result = get_node_set(doc, path);
  if(result){
    nodeset = result->nodesetval;
    keyword = xmlGetProp(nodeset->nodeTab[0], (xmlChar*) "src");
  }

  // Image not found in page, return failure image;
  if(keyword == NULL){
    xmlFreeDoc(doc);
    xmlCleanupParser();
    delete pageMem;
    return Screen::load_stored_texture(0);
  }

  MemoryStruct* image = new MemoryStruct;
  ApiManager::get_res(image, (char*) keyword);
  xmlFree(keyword);
  xmlFreeDoc(doc);
  xmlCleanupParser();
  delete pageMem;
  // Clean up existing image

  // Render new page, if fails, use stock failure image
  SDL_Texture* ret = Screen::load_texture(image->memory, image->size);

  delete image;
  return ret;
}

Handler GalleryBrowser::on_event(int val){
  // Next page
  if(val == 1 && cur_page < (active_gallery->total_pages - 1)){
    cur_page++;

    Screen::cleanup_texture(prev_image);
    prev_image = active_image;
    active_image = next_image;
    // Not last page, load next
    if(cur_page < active_gallery->total_pages - 1)
      next_image = load_page(cur_page+1);
    // If not set to NULL, will wipe on prev
    else
      next_image = NULL;
  }

  // Previous page
  if(val == 2 && cur_page > 0){
    cur_page--;

    Screen::cleanup_texture(next_image);
    next_image = active_image;
    active_image = prev_image;
    // Not first page, load prev
    if(cur_page > 0)
      prev_image = load_page(cur_page-1);
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
  // Image
  Screen::draw_adjusted_mem(active_image, 100, 0, screen_width - 100, screen_height);
  // Right side button (next and kill)
  Screen::draw_button(screen_width-90, (screen_height/2) - 40, 80, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);
  // Left side button (prev)
  Screen::draw_button(10, (screen_height/2) - 40, 80, 80, ThemeButton, ThemeButtonBorder, 4);
}
