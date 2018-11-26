#include "Gallery.hpp"
#include "Config.hpp"
#include "Touch.hpp"
#include "Api.hpp"
#include "Ui.hpp"
#include "Browser.hpp"
#include <cstdlib>

#define pagesXPath "//a[contains(@href, 'hentai.org/s/')]"
#define imageXPath "//img[@id='img']"

Gallery* GalleryBrowser::active_gallery;
int GalleryBrowser::cur_page = 0;
const int GalleryBrowser::buffer_size = 2; // 1 - 1 Page, 2 - 3 pages, 3 - 5 pages...
std::vector<Resource*> GalleryBrowser::img_buffer;
static int rotation = 0;
static int block_size;

static float oldZoomFactor = 1;
static float zoomFactor = 1;
static SDL_Point pos;
static SDL_Point startPos;

// Sets image bounds to keep onscreen when zooming
void set_pos_bounds();

void GalleryBrowser::close(){
  if(active_gallery){
    delete active_gallery;
  }

  for(auto res : img_buffer){
    ApiManager::cleanup_resource(res);
  }

  img_buffer.clear();
}

// Load gallery
void GalleryBrowser::load_gallery(Entry* entry){
  block_size = 1;
  // Cur image load
  zoomFactor = 1;
  pos.x = 0;
  pos.y = 0;

  printf("Loading Gallery %s\n", entry->url.c_str());

  rotation = atoi(ConfigManager::get_value("rotation").c_str());

  // Debug readout
  printf("-- LOADING GALLERY --\nTitle: %s\nURL: %s\nPages:%d\n", entry->title.c_str(), entry->url.c_str(), entry->pages);
  active_gallery = new Gallery();
  active_gallery->title = entry->title;
  active_gallery->index = entry->url;
  active_gallery->total_pages = entry->pages;

  // Create all buffer pages
  for(int i = 0; i < active_gallery->total_pages; i++){
    Resource* res = new Resource;
    img_buffer.push_back(res);
  }

  // Populate image buffer area
  for(int i = 0; i < active_gallery->total_pages && i < buffer_size; i++){
    GalleryBrowser::load_page(i);
  }

  // Set current page to start of gallery
  cur_page = 0;
}

// Set up touch controls
void GalleryBrowser::set_touch(){
  TouchManager::clear();

  // Browser
  TouchManager::add_bounds(screen_width - 75, 0, 75, 75, 101);

  // Portrait
  if(rotation == 1){
    // Forwards
    TouchManager::add_bounds(0, screen_height * 0.7, screen_width, screen_width * 0.3, 104);

    // Back
    TouchManager::add_bounds(0, 0, screen_width, screen_height * 0.3, 103);
  } 
  // Landscape
  else {
    // Forwards
    TouchManager::add_bounds(screen_width * 0.7, 0, screen_width * 0.3, screen_height, 104);

    // Back
    TouchManager::add_bounds(0, 0, screen_width * 0.3, screen_height, 103);
  }
}

void GalleryBrowser::load_page(int page){
  Resource* res = img_buffer[page];

  // If resource already has a URL set, skip finding src, request immediately
  if(res->populated){
    printf("Requesting page %d\n", page);
    ApiManager::request_res(res);
    return;
  }

  // Load more URLs if needed - block_size on each page
  if(page >= (int) active_gallery->pages.size()){
    int block = (active_gallery->pages.size()/block_size);
    printf("Loading page block %d\n", block);
    load_urls(block);
  }

  // Still not loaded, failed to load gallery
  if(page >= (int) active_gallery->pages.size()){
    img_buffer[page]->texture = Screen::load_stored_texture(0);
    return;
  }

  xmlChar *path;
  xmlChar *keyword = NULL;
  xmlXPathObjectPtr result;
  xmlDocPtr doc;
  xmlNodeSetPtr nodeset;

  // Get html page
  MemoryStruct* pageMem = new MemoryStruct();
  ApiManager::get_res(pageMem, active_gallery->pages[page].c_str());

  // If page failed to load, return failure image
  if(pageMem->size == 0){
    delete pageMem;
    img_buffer[page]->texture = Screen::load_stored_texture(0);
  }

  doc = htmlReadMemory(pageMem->memory, pageMem->size, active_gallery->index.c_str(), NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

  if(doc == nullptr){
    xmlCleanupParser();
    delete pageMem;
    return;
  }

  // Get node list matching XPath for direct image url
  path = (xmlChar*) imageXPath;
  result = get_node_set(doc, path);
  if(result){
    nodeset = result->nodesetval;
    keyword = xmlGetProp(nodeset->nodeTab[0], (xmlChar*) "src");
  }

  // Load URL into resource, then request
  if(keyword){
    printf("Requesting page %d\n", page);
    res->url = (char*)keyword;
    res->populated = 1;
    ApiManager::request_res(res);
  }

  xmlFree(keyword);
  xmlFreeDoc(doc);
  xmlCleanupParser();
  delete pageMem;
}

HandlerEnum GalleryBrowser::on_event(int val){
  // Next page
  if(val == 104 && cur_page < (active_gallery->total_pages - 1)){
    int to_add = cur_page + buffer_size;
    cur_page++;
    int to_remove = cur_page - buffer_size;

    // Immediate page switch
    zoomFactor = 1;
    pos.x = 0;
    pos.y = 0;
    render();

    // Unload oldest page
    if(to_remove >= 0){
      Resource* removing = img_buffer[to_remove];
      removing->requested = 0;

      if(removing->texture)
        Screen::cleanup_texture(img_buffer[to_remove]->texture);
      removing->texture = NULL;
    }

    // Load furthest page
    if(to_add <= active_gallery->total_pages - 1){
      GalleryBrowser::load_page(to_add);
    }
  }

  // Previous page
  if(val == 103 && cur_page > 0){
    int to_add = cur_page - buffer_size;
    cur_page--;
    int to_remove = cur_page + buffer_size;

    // Immediate page switch
    zoomFactor = 1;
    pos.x = 0;
    pos.y = 0;
    render();

    // Unload oldest page
    if(to_remove <= active_gallery->total_pages - 1){
      Resource* removing = img_buffer[to_remove];
      removing->requested = 0;

      if(removing->texture)
        Screen::cleanup_texture(img_buffer[to_remove]->texture);
      removing->texture = NULL;
    }

    // Load furthest page
    if(to_add >= 0){
      GalleryBrowser::load_page(to_add);
    }
  }

  // Back to browser
  if(val == 101){
    ConfigManager::set_pair("rotation", std::to_string(rotation));
    ApiManager::cancel_all_requests();
    GalleryBrowser::close();
    Browser::set_touch();
    return HandlerEnum::Browser;
  }

  // Rotate Image
  if(val == 110){
    rotation++;
    if(rotation > 1)
      rotation = 0;
    GalleryBrowser::set_touch();
  }

  return HandlerEnum::Gallery;
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
  ApiManager::get_res(index, indexCopy);

  if(index->size == 0){
    delete index;
    return;
  }

  // Load to xml
  xmlDocPtr doc = htmlReadMemory(index->memory, index->size, active_gallery->index.c_str(), NULL, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);

  if(doc == nullptr){
    xmlCleanupParser();
    return;
  }

  // Get node list matching XPath
  xmlXPathObjectPtr result = get_node_set(doc, path);

  // Iterate through results
  if(result) {
    xmlNodeSetPtr nodeset = result->nodesetval;
    printf("Found %d entries\n", nodeset->nodeNr);
    if(block_size == 1)
      block_size = nodeset->nodeNr;

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

void GalleryBrowser::save_all_pages(std::string dir){
  // Load all URLs
  int url_page = 1;
  while((int) active_gallery->pages.size() < active_gallery->total_pages){
    load_urls(url_page);
    url_page++;
  }

  for(int page = 0; page < (int) active_gallery->pages.size(); page++){
    // Update progress
    int progress = ((float) page / (float) active_gallery->total_pages) * ((screen_width / 2) - 10);

    Screen::clear(ThemeBG);
    Screen::draw_text_centered("Downloading Gallery...", 0, (screen_height / 2) - 100, screen_width, 100, ThemeText, Screen::large);
    Screen::draw_rect(screen_width / 4, screen_height / 2, screen_width / 2, 150, ThemePanelDark);
    Screen::draw_rect(screen_width / 4 + 5, (screen_height / 2) + 5, progress, 140, ThemePanelLight);
    Screen::render();

    xmlChar *path;
    xmlChar *keyword = NULL;
    xmlXPathObjectPtr result;
    xmlDocPtr doc;
    xmlNodeSetPtr nodeset;

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

    // Save image
    if(keyword){
      printf("Saving page %d\n", page);
      ApiManager::get_res(NULL, (char*) keyword, ApiManager::handle, 1, dir + "/page" + std::to_string(page) + ".jpg");
    }

    xmlFree(keyword);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    delete pageMem;

  }
}

void GalleryBrowser::render(){
  Screen::clear(ThemeBG);
  // Image (If loaded)
  if(img_buffer[cur_page]->texture){
    Screen::draw_adjusted_mem(img_buffer[cur_page]->texture, pos.x, pos.y, screen_width * zoomFactor, screen_height * zoomFactor, rotation);
  }

  // Page Number
  Screen::draw_text("Page " + std::to_string(cur_page+1), 30, 30, ThemeText, Screen::large);
  // Right side button (next and kill)
  //Screen::draw_button(screen_width-90, (screen_height/2) - 40, 80, 80, ThemeButton, ThemeButtonBorder, 4);
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);
  // Left side button (prev)
  //Screen::draw_button(10, (screen_height/2) - 40, 80, 80, ThemeButton, ThemeButtonBorder, 4);
}

void GalleryBrowser::set_pos_bounds(){
  int realWidth = screen_width * zoomFactor;
  int realHeight = screen_height * zoomFactor;
  int w, h;
  SDL_QueryTexture(img_buffer[cur_page]->texture, NULL, NULL, &w, &h);
  float scaler;
  // Distance from edge, nothing, width, height
  SDL_Point rect;

  if(!(rotation % 2)){
    scaler = realHeight / ((float)(h));
    rect.x = (int)(w * scaler);
    rect.y = (int)(h * scaler);

    scaler = realWidth / ((float)(rect.x));
    if(scaler < 1){
      rect.x *= scaler;
      rect.y *= scaler;
    }
  } else {
    scaler = realHeight / ((float)(w));
    rect.x = (int)(w * scaler);
    rect.y = (int)(h * scaler);

    scaler = realWidth / ((float)(rect.y));
    if(scaler < 1){
      rect.x *= scaler;
      rect.y *= scaler;
    }
  }


  float distFromEdge;
  if(rotation)
    distFromEdge = (realWidth - rect.y) / 2;
  else
    distFromEdge = (realWidth - rect.x) / 2;

  int bottomBound = 0;
  int topBound = screen_height - realHeight;
  int leftBound, rightBound;

  if(rect.x < screen_width){
    rightBound = realWidth - screen_width + (distFromEdge); 
    leftBound = 0 - distFromEdge;
  } else {
    rightBound = 0 - distFromEdge;
    leftBound = screen_width - (realWidth - distFromEdge);
  }

  printf("Bounds {%d, %d, %d, %d}\n", bottomBound, topBound, rightBound, leftBound);
  printf("Pos {%d, %d}\n", pos.x, pos.y);

  if(pos.x < leftBound)
    pos.x = leftBound;

  if(pos.y < topBound)
    pos.y = topBound;

  if(pos.x > rightBound)
    pos.x = rightBound;

  if(pos.y > bottomBound)
    pos.y = bottomBound;

  printf("Pos Adjusted {%d, %d}\n", pos.x, pos.y);
}

void GalleryBrowser::gesture(SDL_Event e) {
  printf("Doing Gesture\n");
  // First finger, second first, original first pos, original second pos
  SDL_Finger* first = SDL_GetTouchFinger(e.mgesture.touchId, 0);
  SDL_Finger* second = SDL_GetTouchFinger(e.mgesture.touchId, 1);

  // Catch missing fingers exceptions
  if(first == NULL || second == NULL)
    return;

  FloatPoint firstStart = fingerTouches[first->id];
  FloatPoint secondStart = fingerTouches[second->id];

  // Get distances between each pair of points, calc percentage change in zoom
  float distStart = pow((firstStart.x*screen_width) - (secondStart.x*screen_width), 2) + pow((firstStart.y*screen_height) - (secondStart.y*screen_height), 2);
  float distNow = pow((first->x*screen_width) - (second->x*screen_width), 2) + pow((first->y*screen_height) - (second->y*screen_height), 2);
  float percentChange = distNow / distStart;

  // Cap change in zoom - Stops infinite size zooms
  if(percentChange < 0.1)
    percentChange = 0.1;
  if(percentChange > 4)
    percentChange = 4; 

  zoomFactor = oldZoomFactor * percentChange;
  printf("New %f, Old %f\n", zoomFactor, oldZoomFactor);

  // Cap zoom level
  if(zoomFactor > 4)
    zoomFactor = 4;
  if(zoomFactor < 1)
    zoomFactor = 1;


  // Translate to percentage img coords
  float pX = abs(startPos.x - (e.mgesture.x * screen_width)) / (screen_width * oldZoomFactor);
  float pY = abs(startPos.y - (e.mgesture.y * screen_height)) / (screen_height * oldZoomFactor);
  printf("Percent of image {%f, %f}\n", pX, pY);

  // Get screen amount from percentage, take away from midpoint to get real coords
  pos.x = (e.mgesture.x * screen_width) - (pX * screen_width * zoomFactor);
  pos.y = (e.mgesture.y * screen_height) - (pY * screen_height * zoomFactor);
  printf("Translated {%d, %d}\n", pos.x, pos.y);

  set_pos_bounds();
}

void GalleryBrowser::scroll(float dx, float dy){
  pos.x += dx * screen_width;
  pos.y += dy * screen_height;
  set_pos_bounds();
}

void GalleryBrowser::finger_down(SDL_Event e){
  if(fingerTouches.size() >= 2){
    oldZoomFactor = zoomFactor;
    printf("Set old %f\n", oldZoomFactor);
    startPos = pos;
  }
}

void GalleryBrowser::finger_up(SDL_Event e){
}
