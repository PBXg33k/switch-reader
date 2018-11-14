#include "Settings.hpp"
#include "Touch.hpp"
#include "Ui.hpp"
#include "Config.hpp"
#include "Browser.hpp"
#include <math.h>

void Settings::set_touch(){
    Screen::clear(ThemeBG);

    TouchManager::clear();

    // Browser
    TouchManager::add_bounds(screen_width - 75, 0, 75, 75, 101);

    // Theme
    TouchManager::add_bounds(50, 50, 300, 120, 1);
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
    }

    // Return to Browser
    if(val == 101){
        ConfigManager::save();
        Browser::set_touch();
        return Handler::Browser;
    }
    return Handler::Settings;
}

void Settings::render(){
  Screen::clear(ThemeBG);
  // Theme
  Screen::draw_button(50, 50, 300, 120, ThemeButton, ThemeButtonBorder, 5);
  Screen::draw_text_centered(ConfigManager::get_value("theme"), 50, 50, 300, 120, ThemeButtonText, Screen::large);

  // Quit Button
  Screen::draw_button(screen_width - 75, 0, 75, 75, ThemeButtonQuit, ThemeButtonBorder, 4);
}