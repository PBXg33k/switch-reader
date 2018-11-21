#include "Settings.hpp"
#include "Touch.hpp"
#include "Ui.hpp"
#include "Config.hpp"
#include "Keyboard.hpp"
#include "Browser.hpp"
#include <math.h>

static int field = 0;
static std::string username;
static std::string password;
static std::string proxy;

void Settings::set_touch(){
    proxy = ApiProxy;

    Screen::clear(ThemeBG);

    TouchManager::clear();

    // Browser
    TouchManager::add_bounds(screen_width - 75, 0, 75, 75, 101);

    // Theme
    TouchManager::add_bounds(50, 50, 300, 120, 1);

    // Exhentai
    TouchManager::add_bounds(50, 190, 300, 120, 2);
    
    // Username
    TouchManager::add_bounds(250, 330, 680, 120, 3);

    // Password
    TouchManager::add_bounds(250, 470, 680, 120, 4);

    // Proxy
    TouchManager::add_bounds(600, 100, 680, 120, 5);
}

Handler Settings::on_event(int val){

    // Theme
    if(val == 1){
      std::string theme = ConfigManager::get_value("theme");
      
      int new_theme = stoi(theme);
      printf("Current theme %d\n", new_theme);
      if(new_theme == 1)
          new_theme = 0;
      else
          new_theme = 1;

      theme = std::to_string(new_theme);
      printf("Set Theme %s\n", theme.c_str());

      ConfigManager::set_pair("theme", theme);
      ConfigManager::set_theme();
      printf("New Theme %s\n", ConfigManager::get_value("theme").c_str());
    // ExHentai toggle
    } else if(val == 2){
      std::string mode = ConfigManager::get_value("mode");
      if(mode == "E-hentai")
        mode = "Exhentai";
      else
        mode = "E-hentai";
      ConfigManager::set_pair("mode", mode);
      ConfigManager::set_mode();
    // Username
    } else if(val == 3){
      field = 0;
      Keyboard::setup(Handler::Settings);
      Keyboard::set_touch();
      return Handler::Keyboard;
    // Password
    } else if(val == 4){
      field = 1;
      Keyboard::setup(Handler::Settings);
      Keyboard::set_touch();
      return Handler::Keyboard;
    } else if(val == 5){
      field = 2;
      Keyboard::setup(Handler::Settings);
      Keyboard::set_touch();
      Keyboard::text = proxy;
      return Handler::Keyboard;
    }

    if(val == Shared::KeyboardReturn){
      set_touch();

      switch(field){
        case 0:
          username = Keyboard::text;
          break;
        case 1:
          password = Keyboard::text;
          break;
        case 2:
          proxy = Keyboard::text;
          ConfigManager::set_pair("proxy", proxy);
          ConfigManager::set_proxy();
          break;
        default:
          break;
      }


    }

    // Return to Browser
    if(val == 101){
      if(!username.empty() && !password.empty())
        ApiManager::login(username, password);

      // Don't keep in memory
      username.clear();
      password.clear();

      Browser::load_username();

      // Save config file
      ConfigManager::save();

      // Return
      Browser::set_touch();
      return Handler::Browser;
    }
    return Handler::Settings;
}

void Settings::render(){
  Screen::clear(ThemeBG);
  // Theme
  int theme = stoi(ConfigManager::get_value("theme"));
  std::string themeStr;
  switch(theme){
    case 0:
      themeStr = "Light Theme";
      break;
    case 1:
      themeStr = "Dark Theme";
      break;
    default:
      themeStr = "? Theme";
      break;
  }

  // Theme button
  Screen::draw_button(50, 50, 300, 120, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text_centered(themeStr.c_str(), 50, 50, 300, 120, ThemeButtonText, Screen::large);

  // Exhentai toggle
  Screen::draw_button(50, 190, 300, 120, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text_centered(ConfigManager::get_value("mode").c_str(), 50, 190, 300, 120, ThemeButtonText, Screen::large);

  // Username
  Screen::draw_button(250, 330, 680, 120, ThemeButton, ThemeButtonBorder, 6);
  Screen::draw_text("Username", 50, 360, ThemeText, Screen::large);
  if(!username.empty())
    Screen::draw_text_centered(username.c_str(), 250, 330, 680, 120, ThemeText, Screen::large);

  // Password
  Screen::draw_button(250, 470, 680, 120, ThemeButton, ThemeButtonBorder, 6);
  Screen::draw_text("Password", 50, 500, ThemeText, Screen::large);
  if(!password.empty())
    Screen::draw_text_centered(password.c_str(), 250, 470, 680, 120, ThemeText, Screen::large);

  // Proxy
  Screen::draw_button(600, 100, 680, 120, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text("Proxy URL", 400, 130, ThemeText, Screen::large);
  if(!proxy.empty())
    Screen::draw_text_centered(proxy.c_str(), 600, 100, 680, 120, ThemeText, Screen::large);

  // Quit Button
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);
}