#include "Settings.hpp"
#include "Touch.hpp"
#include "Ui.hpp"
#include "Config.hpp"
#include "HSearch.hpp"
#include "Keyboard.hpp"
#include "Search.hpp"
#include "Browser.hpp"
#include "Dialog.hpp"

#include <math.h>

static std::string username;
static std::string password;
static int press_sens = -1;

void Settings::set_touch(){
  Screen::clear(ThemeBG);

  TouchManager::instance.clear();

  // Browser
  TouchManager::instance.add_bounds(screen_width - 75, 0, 75, 75, 101);

  // Theme
  TouchManager::instance.add_bounds(50, 560, 300, 120, 1);

  // Mode
  TouchManager::instance.add_bounds(370, 560, 300, 120, 2);
  
  // Proxy
  TouchManager::instance.add_bounds(250, 50, 590, 120, 5);

  // Username
  TouchManager::instance.add_bounds(250, 190, 590, 120, 3);

  // Password
  TouchManager::instance.add_bounds(250, 330, 590, 120, 4);

  // Add Szuru
  TouchManager::instance.add_bounds(860, 190, 330, 120, 10);

  HSearch::current_domain()->settings_touch();
}

HandlerEnum Settings::on_event(int val){

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
  // Mode switch
  } else if(val == 2){
    std::vector<std::string> choices;
    std::string message = "Pick a Domain";

    // Add all domains as choices
    for(auto pair : HSearch::get_domains()){
      choices.push_back(pair.first);
    }

    int res = Dialog::get_response(message, choices);
    if(res < 100){
      std::string domain = HSearch::get_domains().find(choices[res])->first;
      ConfigManager::set_pair("domain", domain);
    }

  // Username
  } else if(val == 3){
    username = Keyboard::get_input("Username", username);
  // Password
  } else if(val == 4){
    password = Keyboard::get_input("Password", password);
  } else if(val == 5){
    ConfigManager::set_pair("proxy", Keyboard::get_input("Proxy URL", ApiProxy));
    ConfigManager::set_proxy();
  } else if(val == 10){
    std::string name = Keyboard::get_input("Name");
    std::string domain = Keyboard::get_input("Domain URL");
    ConfigManager::add_pair("szuru_domain", name + "," + domain);
    HSearch::register_domain(name, new Domain_Szuru(domain, name));
  }

  // Return to Browser
  if(val == 101){
    if(!username.empty() && !password.empty()){
      Screen::clear(ThemeBG);
      Screen::draw_text_centered("Logging in...", 0, 0, screen_width, screen_height, ThemeText, Screen::header);
      Screen::render();
      Domain* domain = HSearch::current_domain();
      domain->login(username, password);
    }

    // Don't keep in memory
    username.clear();
    password.clear();

    Browser::load_username();
    Browser::clear();

    ConfigManager::set_pair("press_sens", std::to_string(press_sens));

    // Save config file
    ConfigManager::save();

    // Do search
    HSearch::search_keywords(SearchBrowser::search_str);

    // Return
    Browser::set_touch();
    return HandlerEnum::Browser;
  }
  
  return HSearch::current_domain()->settings_event(val);
}


void Settings::render(){
  Screen::clear(ThemeBG);
  // Theme
  int theme = stoi(ConfigManager::get_value("theme"));
  std::string themeStr;
  switch(theme){
    case 0:
      themeStr = "Light";
      break;
    case 1:
      themeStr = "Dark";
      break;
    default:
      themeStr = "?";
      break;
  }

  // Load sens if not already
  if(press_sens == -1)
    press_sens = stoi(ConfigManager::get_value("press_sens"));

  // Proxy
  Screen::draw_button(250, 50, 590, 120, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text("Proxy URL", 50, 80, ThemeText, Screen::large);
  if(!ApiProxy.empty())
    Screen::draw_text_centered(ApiProxy.c_str(), 250, 50, 590, 120, ThemeText, Screen::large);

  // Username
  Screen::draw_button(250, 190, 590, 120, ThemeButton, ThemeButtonBorder, 6);
  Screen::draw_text("Username", 50, 220, ThemeText, Screen::large);
  if(!username.empty())
    Screen::draw_text_centered(username.c_str(), 250, 190, 590, 120, ThemeText, Screen::large);

  // Password
  Screen::draw_button(250, 330, 590, 120, ThemeButton, ThemeButtonBorder, 6);
  Screen::draw_text("Password", 50, 360, ThemeText, Screen::large);
  if(!password.empty())
    Screen::draw_text_centered(password.c_str(), 250, 330, 590, 120, ThemeText, Screen::large);

  // Theme
  Screen::draw_text_centered("Theme", 50, 510, 300, 50, ThemeText, Screen::large);
  Screen::draw_button(50, 560, 300, 120, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text_centered(themeStr.c_str(), 50, 560, 300, 120, ThemeButtonText, Screen::large);

  // Domain
  Screen::draw_text_centered("Domain", 370, 510, 300, 50, ThemeText, Screen::large);
  Screen::draw_button(370, 560, 300, 120, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text_centered(ConfigManager::get_value("domain").c_str(), 370, 560, 300, 120, ThemeButtonText, Screen::large);

  // Sensitivity slider
  Screen::draw_text_centered("Press Sensitivity", 720, 510, 300, 50, ThemeText, Screen::large);
  Screen::draw_rect(720, 620 - 5, 300, 10, ThemeButtonBorder);
  Screen::draw_button(720 + press_sens - 25, 620 - 25, 50, 50, ThemeButton, ThemeButtonBorder, 2);

  // Szuru Controls

  // Add Szuru
  Screen::draw_button(860, 190, 330, 120, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text_centered("Add Szuru Domain", 860, 190, 330, 120, ThemeText, Screen::large);

  // Quit Button
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);

  HSearch::current_domain()->settings_render();
}

void Settings::finger_down(SDL_Event e){
  int x = e.tfinger.x * screen_width;
  int y = e.tfinger.y * screen_height;

  // Press sens
  if(x > 720 && x < 720 + 300){
    if(y > 560 && y < 560 + 120){
      press_sens = x - 720;
    }
  }

}