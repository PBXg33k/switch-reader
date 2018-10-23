#include <stdio.h>

#include "ui.h"
#include "shared.h"
#include "api.h"
#include "browser.h"

int main(int argc, char **argv)
{

  ApiManager::init();
  Screen::init();

  json_object* json = ApiManager::get_gallery((char*)"618395",(char*)"0439fa3666");
  struct Entry entry = Browser::add_entry(json, 0);
  Browser::render_entry(entry, 30, 30);

  //Screen::draw_image("/switch/res", 0, 0);
  //Screen::draw_text(entry.thumb,15,15,COLOR_WHITE);

  Screen::render();

  getchar();

  Screen::close();

  ApiManager::close();

  return 0;
}
