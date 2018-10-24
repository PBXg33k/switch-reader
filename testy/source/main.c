#include <stdio.h>

#include "ui.h"
#include "shared.h"
#include "api.h"
#include "browser.h"
#include "h_search.h"

int main(int argc, char **argv)
{

  ApiManager::init();
  Screen::init();

  std::vector<Entry> result = HSearch::search_keywords("may", 3);
  Browser::render_entry(result[0], 30, 30);
  Browser::render_entry(result[1], 430, 30);
  Browser::render_entry(result[2], 830, 30);

  //Screen::draw_image("/switch/res", 0, 0);
  //Screen::draw_text(entry.thumb,15,15,COLOR_WHITE);

  Screen::render();

  getchar();

  Screen::close();

  ApiManager::close();

  return 0;
}
