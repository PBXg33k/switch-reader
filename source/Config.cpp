#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include "Config.hpp"

#define configDir "/switch/Reader"
#define configPath "/switch/Reader/config"

static std::vector<std::pair<std::string, std::string>> configPairs;

void create_config_default(){
  configPairs.push_back(std::make_pair("theme","light"));
  configPairs.push_back(std::make_pair("rotation","0"));
  configPairs.push_back(std::make_pair("user", "NONE"));
  configPairs.push_back(std::make_pair("pass", "NONE"));
  ConfigManager::save();
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
    return 0;
  }

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

  return 0;
}

void ConfigManager::save(){
  std::ofstream configFile;
  configFile.open(configPath);

  if(!configFile.is_open())
    return;

  printf("Saved config file\n");

  for(auto pair : configPairs){
    configFile << pair.first + "=" + pair.second + "\n";
  }
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
