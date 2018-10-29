#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include "Config.hpp"
#include "Shared.hpp"

#define configDir "/switch/Reader"
#define configPath "/switch/Reader/config"

static std::vector<std::pair<std::string, std::string>> configPairs;

// Create default, clear pairs to check read back
void create_config_default(){
  configPairs.push_back(std::make_pair("theme","0")); // 0 - Light, 1 - Dark
  configPairs.push_back(std::make_pair("rotation","0"));
  configPairs.push_back(std::make_pair("user", "NONE"));
  configPairs.push_back(std::make_pair("pass", "NONE"));
  ConfigManager::save();
  configPairs.clear();
}

int ConfigManager::init(){

  // Make sure subdirectory exists
  struct stat info;
  stat(configDir, &info);
  if(!(info.st_mode & S_IFDIR)){
    printf("Creating config directory\n");
    mkdir(configDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }

  // If config missing, create default
  if(stat(configPath, &info)){
    printf("Creating default config file\n");
    create_config_default();
  }

  // Read back config
  std::ifstream configFile (configPath);
  if (configFile.is_open()){
    printf("Loading config file\n");
    std::string line;
    // Read each line as a key=value, store in pairs
    while(getline(configFile, line)){
      line.erase(std::remove_if(line.begin(), line.end(), isspace),
                                 line.end());
      if(line[0] == '#' || line.empty())
          continue;
      auto delimiterPos = line.find("=");
      auto key = line.substr(0, delimiterPos);
      auto value = line.substr(delimiterPos + 1);

      printf("Loaded pair ( %s , %s )\n", key.c_str(), value.c_str());
      configPairs.push_back(std::make_pair(key, value));
    }
  } else {
    // Failed to load config
    printf("Failed to load config\n");
    return 1;
  }

  configFile.close();

  set_theme(stoi(get_value("theme")));

  return 0;
}

// Save config file
void ConfigManager::save(){
  std::ofstream configFile;
  configFile.open(configPath);

  if(!configFile.is_open())
    return;

  for(auto pair : configPairs){
    configFile << pair.first + "=" + pair.second + "\n";
  }

  configFile.close();
  printf("Saved config file\n");
}

void ConfigManager::set_pair(std::string key, std::string value){

}

std::string ConfigManager::get_value(std::string key){
  for(auto pair : configPairs){
    if(pair.first == key){
      return pair.second;
    }
  }
  return NULL;
}

void ConfigManager::set_theme(int theme){
  switch(theme){
    // Light
    case 0:
      ThemeBG = COLOR_PALEYELLOW;
      ThemeButton = COLOR_PALEWHITE;
      ThemeButtonQuit = COLOR_RED;
      ThemeButtonText = COLOR_LIGHTGRAY;
      ThemeButtonBorder = COLOR_LIGHTGRAY;
      ThemePanelDark = COLOR_PALEISHYELLOW;
      ThemePanelLight = COLOR_PALEWHITE;
      ThemePanelSelectedDark = COLOR_PALEISHRED;
      ThemePanelSelectedLight = COLOR_PALERED;
      ThemeText = COLOR_GRAY;
      ThemeOptionSelected = COLOR_PALEWHITE;
      ThemeOptionUnselected = COLOR_VLIGHTGRAY;
      break;
    // Dark
    case 1:
      ThemeBG = COLOR_SHADEDBLACK;
      ThemeButton = COLOR_SHADEDBLACK;
      ThemeButtonQuit = COLOR_RED;
      ThemeButtonText = COLOR_YELLOWWHITE;
      ThemeButtonBorder = COLOR_TRUEGRAY;
      ThemePanelDark = COLOR_DARKISH;
      ThemePanelLight = COLOR_PALEBLACK;
      ThemePanelSelectedDark = COLOR_PALEISHRED;
      ThemePanelSelectedLight = COLOR_PALERED;
      ThemeText = COLOR_SUPERGRAY;
      ThemeOptionSelected = COLOR_PALEBLACK;
      ThemeOptionUnselected = COLOR_SHADEDBLACK;
      break;
    default:
      break;
  }
}