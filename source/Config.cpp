#include <map>
#include <string>
#include <utility>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include "Config.hpp"
#include "Shared.hpp"
#include "HSearch.hpp"

#define configDir "/switch/Reader"
#define configPath "/switch/Reader/config"

#define themeDefault "1"
#define rotationDefault "0"
#define pressDefault "150"
#define proxyDefault "http://192.168.0.123:5000/?url=" 
#define categoriesDefault Category::NonH

static std::map<std::string, std::string> configPairs;

void load_defaults(){
  configPairs.insert(std::make_pair("theme", themeDefault));
  configPairs.insert(std::make_pair("rotation", rotationDefault));
  configPairs.insert(std::make_pair("domain", HSearch::get_domains().begin()->first));
  configPairs.insert(std::make_pair("proxy", proxyDefault));
  configPairs.insert(std::make_pair("press_sens", pressDefault));
  configPairs.insert(std::make_pair("lang", ""));
  configPairs.insert(std::make_pair("stars","1"));
  configPairs.insert(std::make_pair("search", "Public"));
  configPairs.insert(std::make_pair("categories", std::to_string((int) categoriesDefault)));
}

// Create default, clear pairs to check read back
void create_config_default(){
  load_defaults();
  ConfigManager::save();
  configPairs.clear();
}

// Update old config files by finding missing pairs
void update_config(){
  if(!configPairs.count("press_sens"))
    ConfigManager::set_pair("press_sens", pressDefault);

  if(!configPairs.count("domain"))
    ConfigManager::set_pair("domain", HSearch::get_domains().begin()->first);

  if(!configPairs.count("lang"))
    ConfigManager::set_pair("lang", "");

  if(!configPairs.count("stars"))
    ConfigManager::set_pair("stars", "1");

  if(!configPairs.count("search"))
    ConfigManager::set_pair("search", "Public");

  if(!configPairs.count("categories"))
    ConfigManager::set_pair("categories", std::to_string((int) categoriesDefault));
}

int ConfigManager::init(){

  // Make sure subdirectory exists
  struct stat info;
  struct stat dInfo;
  struct stat cInfo;

  stat(configDir, &info);
  if(!(info.st_mode & S_IFDIR)){
    printf("Creating config directory\n");
    mkdir(configDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }

  stat(downloadsDir.c_str(), &dInfo);
  if(!(dInfo.st_mode & S_IFDIR)){
    printf("Creating downloads directory\n");
    mkdir(downloadsDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }
  
  // If config missing, create default
  if(stat(configPath, &cInfo)){
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
      configPairs.insert(std::make_pair(key, value));
    }
  } else {
    // Failed to load config
    printf("Failed to load config, using defaults\n");
    load_defaults();
    return 1;
  }

  configFile.close();
  configFile.clear();

  update_config();
  set_all();

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
  configFile.clear();

  printf("Saved config file\n");
}

void ConfigManager::set_pair(std::string key, std::string value){
  auto item = configPairs.find(key);

  if(item != configPairs.end()){
    item->second = value;
  } else {
    configPairs.insert(std::make_pair(key, value));
  }
}

std::string ConfigManager::get_value(std::string key){
  auto item = configPairs.find(key);

  if(item != configPairs.end()){
    return item->second;
  }

  return "";
}

void ConfigManager::set_theme(){
  std::string themeStr = get_value("theme");
  
  // No theme set, use default
  if(themeStr.empty()){
    themeStr = themeDefault;
    set_pair("theme", themeStr);
  }

  int theme = stoi(themeStr);

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

void ConfigManager::set_proxy(){
  std::string proxy = get_value("proxy");

  // No proxy set, make default
  if(proxy.empty()){
    proxy = proxyDefault;
    set_pair("proxy", proxy);
  }

  ApiProxy = proxy;
}

void ConfigManager::set_all(){
  set_theme();
  set_proxy();
}

void ConfigManager::save_entry_info(Entry* entry){
  std::string path = configDir + std::to_string(entry->id) + "/info";
  std::ofstream f;

  f.open(path.c_str());
  f << *entry << std::endl;
  f.close();
}