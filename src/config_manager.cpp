#include "../include/config_manager.hpp"
#include "../include/macros.hpp"
#include "../toml++/toml.h"

ConfigManager* ConfigManager::_instance = nullptr;

void ConfigManager::create_instance() noexcept
{
  if(_instance)
  {
    ERROR_BOII("ConfigManager is already instantiated, use "
               "ConfigManager::get_instance()");
    return;
  }

  _instance = new ConfigManager();
}

ConfigManager* ConfigManager::get_instance() noexcept
{
  return _instance;
}

void ConfigManager::delete_instance() noexcept
{
  delete _instance;
}

bool ConfigManager::load_config(const std::string& config_file_path) noexcept
{
  std::string config_path = "config.toml";

  if(!config_file_path.empty())
  {
    config_path = config_file_path;
  }

  toml::table parsed_config;
  try
  {
    parsed_config = toml::parse_file(config_path);
  }
  catch(const toml::parse_error& error)
  {
    ERROR_BOII("Error while parsing config: %s", error.description());
    return false;
  }

  _config.window.width =
    parsed_config["window"]["width"].value_or<uint16>(1080);
  _config.window.height =
    parsed_config["window"]["height"].value_or<uint16>(720);

  return true;
}

const config& ConfigManager::get_config_struct() const noexcept
{
  return _config;
}