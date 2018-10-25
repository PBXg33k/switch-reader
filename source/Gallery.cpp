#include "Gallery.h"
#include "TouchManager.h"
#include "api.h"
#include "ui.h"
#include "Browser.h"
#include <libxml/HTMLparser.h>

#define pageBlockRegex "class=\"gdtm(.+?)class=\"c"
#define pageListRegex "href=\"(.+?)(?=\"><img alt)"
#define imageRegex "<img id=\"img\" src=\"(.+?)(?=\")"

Gallery* GalleryBrowser::active_gallery;
SDL_Texture* GalleryBrowser::active_image = NULL;

// Load gallery
void GalleryBrowser::load_gallery(Entry* entry){
  printf("Loading gallery\n");
  active_gallery = new Gallery();
  active_gallery->title = entry->title;
  active_gallery->index = entry->url;
  printf("Active url %s Active url 2 %s\n", entry->url.c_str(), active_gallery->index.c_str());
  printf("Loading URLs\n");
  GalleryBrowser::load_urls(0);
  printf("Loading page\n");
  GalleryBrowser::load_page(0);
}


// Set up touch controls
void GalleryBrowser::set_touch(){
  TouchManager::clear();

  // Forwards
  SDL_Rect button = {screen_width-90, (screen_height/2) - 40, screen_width-10, (screen_height/2) + 40};
  TouchManager::add_bounds(button, 1);

  // Back
  button = {10, (screen_height/2) - 40, 90, (screen_height/2) + 40};
  TouchManager::add_bounds(button, 2);

  // Quit
  button = {screen_width-75, 0, screen_width, 75};
  TouchManager::add_bounds(button, 100);
}

void GalleryBrowser::load_page(int page){
  const char* url = active_gallery->pages[page];
  MemoryStruct pageMem = ApiManager::get_res(url);
  std::string pageContent(pageMem.memory);

  std::regex imagePattern(imageRegex);

  std::smatch imageMatch = RegexHelper::search_once(imagePattern, pageContent.c_str());

  MemoryStruct image = ApiManager::get_res(imageMatch[1].str().c_str());
  // Clean up existing image
  if(active_image){
    SDL_DestroyTexture(active_image);
  }
  // Render new one
  active_image = Screen::load_texture(image.memory, image.size);
}

Handler GalleryBrowser::on_event(int val){
  return Handler::Gallery;
}

// Buffer page urls on numbered index page (max 40 per index page)
void GalleryBrowser::load_urls(int page){
  // Load page into memory
  MemoryStruct index = ApiManager::get_res(active_gallery->index.c_str());

  // Load to xml
  htmlDocPtr doc = htmlReadMemory(index.memory, index.size, active_gallery->index.c_str(), NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
  xmlNode *rootElem = xmlDocGetRootElement(doc);
}

void GalleryBrowser::render(){
  // Image
  Screen::draw_adjusted_mem(active_image, 100, 0, screen_width - 100, screen_height);
  // Right side button (next and kill)
  Screen::draw_rect(screen_width-90, (screen_height/2) - 40, 80, 80, COLOR_WHITE);
  Screen::draw_rect(screen_width - 75, 0, 75, 75, COLOR_RED);
  // Left side button (prev)
  Screen::draw_rect(10, (screen_height/2) - 40, 80, 80, COLOR_WHITE);
}
