#include "Gallery.h"
#include "TouchManager.h"
#include "api.h"
#include "ui.h"
#include "Browser.h"


#define pagesXPath "//a[starts-with(@href, 'https://e-hentai.org/s/')]"
#define imageXPath "//img[contains(@id, 'img')]"
#define imageRegex "<img id=\"img\" src=\"(.+?)(?=\")"

Gallery* GalleryBrowser::active_gallery;
SDL_Texture* GalleryBrowser::active_image = NULL;
int GalleryBrowser::cur_page = 0;
bool GalleryBrowser::clear_next_render = false;

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
  cur_page = 0;
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
  // Clear screen
  printf("Loaded page %d\n", page);
  clear_next_render = true;

  // Get html page
  MemoryStruct pageMem = ApiManager::get_res(active_gallery->pages[page].c_str());

  xmlChar *path = (xmlChar*) imageXPath;
  xmlChar *keyword;

  xmlDocPtr doc = htmlReadMemory(pageMem.memory, pageMem.size, active_gallery->index.c_str(), NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
  xmlNodePtr rootElem = xmlDocGetRootElement(doc);

  // Get node list matching XPath for image
  xmlXPathObjectPtr result = get_node_set(doc, path);
  if(result){
    xmlNodeSetPtr nodeset = result->nodesetval;
    keyword = xmlGetProp(nodeset->nodeTab[0], (xmlChar*) "src");

  }

  MemoryStruct image = ApiManager::get_res((char*) keyword);
  // Clean up existing image
  if(active_image){
    SDL_DestroyTexture(active_image);
  }
  // Render new one
  active_image = Screen::load_texture(image.memory, image.size);
}

Handler GalleryBrowser::on_event(int val){
  if(val == 1 && cur_page < (active_gallery->pages.size() - 1)){
    cur_page++;
    load_page(cur_page);
  }
  if(val == 2 && cur_page > 0){
    cur_page--;
    load_page(cur_page);
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
void GalleryBrowser::load_urls(int indexPage){


  xmlChar *path = (xmlChar*) pagesXPath;
  xmlChar *keyword;
  int i;

  // Load page into memory
  MemoryStruct index = ApiManager::get_res(active_gallery->index.c_str());

  // Load to xml
  xmlDocPtr doc = htmlReadMemory(index.memory, index.size, active_gallery->index.c_str(), NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
  xmlNodePtr rootElem = xmlDocGetRootElement(doc);

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

}

void GalleryBrowser::render(){
  if(clear_next_render){
    Screen::clear(COLOR_BLACK);
    clear_next_render = false;
  }
  // Image
  Screen::draw_adjusted_mem(active_image, 100, 0, screen_width - 100, screen_height);
  // Right side button (next and kill)
  Screen::draw_rect(screen_width-90, (screen_height/2) - 40, 80, 80, COLOR_WHITE);
  Screen::draw_rect(screen_width - 75, 0, 75, 75, COLOR_RED);
  // Left side button (prev)
  Screen::draw_rect(10, (screen_height/2) - 40, 80, 80, COLOR_WHITE);
}
