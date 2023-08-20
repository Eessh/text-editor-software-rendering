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

  _config.fps = parsed_config["fps"].value_or<uint8>(60);

  _config.line_numbers_margin =
    parsed_config["line_numbers_margin"].value_or<bool>(true);

  _config.tab_width = parsed_config["tab_width"].value_or<uint8>(2);

  _config.tab_lines = parsed_config["tab_lines"].value_or<bool>(true);

  _config.code_font = parsed_config["code_font"].value_or<std::string>(
    "assets/fonts/JetBrains Mono Regular Nerd Font Complete.ttf");

  _config.font_size = parsed_config["font_size"].value_or<uint8>(14);

  _config.word_separators = parsed_config["word_separators"].value_or<std::string>(" \n\r.!\t;:\\/+-*&%<>=(){}[]\"',|~^");

  _config.window.width =
    parsed_config["window"]["width"].value_or<uint16>(1080);
  _config.window.height =
    parsed_config["window"]["height"].value_or<uint16>(720);

  _config.colorscheme.bg =
    parsed_config["colorscheme"]["bg"].value_or<std::string>("#000000");
  _config.colorscheme.fg =
    parsed_config["colorscheme"]["fg"].value_or<std::string>("#ffffff");
  _config.colorscheme.red =
    parsed_config["colorscheme"]["red"].value_or<std::string>("#ffffff");
  _config.colorscheme.orange =
    parsed_config["colorscheme"]["orange"].value_or<std::string>("#ffffff");
  _config.colorscheme.yellow =
    parsed_config["colorscheme"]["yellow"].value_or<std::string>("#ffffff");
  _config.colorscheme.green =
    parsed_config["colorscheme"]["green"].value_or<std::string>("#ffffff");
  _config.colorscheme.cyan =
    parsed_config["colorscheme"]["cyan"].value_or<std::string>("#ffffff");
  _config.colorscheme.blue =
    parsed_config["colorscheme"]["blue"].value_or<std::string>("#ffffff");
  _config.colorscheme.purple =
    parsed_config["colorscheme"]["purple"].value_or<std::string>("#ffffff");
  _config.colorscheme.white =
    parsed_config["colorscheme"]["white"].value_or<std::string>("#ffffff");
  _config.colorscheme.black =
    parsed_config["colorscheme"]["black"].value_or<std::string>("#ffffff");
  _config.colorscheme.gray =
    parsed_config["colorscheme"]["gray"].value_or<std::string>("#ffffff");
  _config.colorscheme.highlight =
    parsed_config["colorscheme"]["highlight"].value_or<std::string>("#ffffff");
  _config.colorscheme.comment =
    parsed_config["colorscheme"]["comment"].value_or<std::string>("#ffffff");
  _config.colorscheme.scrollbar =
    parsed_config["colorscheme"]["scrollbar"].value_or<std::string>("#ffffff");

  _config.scrolling.sensitivity =
    parsed_config["scrolling"]["sensitivity"].value_or<uint8>(80);
  _config.scrolling.acceleration =
    parsed_config["scrolling"]["acceleration"].value_or<float32>(0.4);

  _config.cpp_token_colors.semicolon =
    parsed_config["cpp_token_colors"]["semicolon"].value_or<std::string>(
      "#ffffff");
  _config.cpp_token_colors.comma =
    parsed_config["cpp_token_colors"]["comma"].value_or<std::string>("#ffffff");
  _config.cpp_token_colors.escape_backslash =
    parsed_config["cpp_token_colors"]["escape_backslash"].value_or<std::string>(
      "#ffffff");
  _config.cpp_token_colors.bracket =
    parsed_config["cpp_token_colors"]["bracket"].value_or<std::string>(
      "#ffffff");
  _config.cpp_token_colors.square_bracket =
    parsed_config["cpp_token_colors"]["square_bracket"].value_or<std::string>(
      "#ffffff");
  _config.cpp_token_colors.curly_bracket =
    parsed_config["cpp_token_colors"]["curly_bracket"].value_or<std::string>(
      "#ffffff");
  _config.cpp_token_colors.character =
    parsed_config["cpp_token_colors"]["character"].value_or<std::string>(
      "#ffffff");
  _config.cpp_token_colors.string =
    parsed_config["cpp_token_colors"]["string"].value_or<std::string>(
      "#ffffff");
  _config.cpp_token_colors.comment =
    parsed_config["cpp_token_colors"]["comment"].value_or<std::string>(
      "#ffffff");
  _config.cpp_token_colors.multiline_comment =
    parsed_config["cpp_token_colors"]["multiline_comment"]
      .value_or<std::string>("#ffffff");
  _config.cpp_token_colors.operator_ =
    parsed_config["cpp_token_colors"]["operator"].value_or<std::string>(
      "#ffffff");
  _config.cpp_token_colors.keyword =
    parsed_config["cpp_token_colors"]["keyword"].value_or<std::string>(
      "#ffffff");
  _config.cpp_token_colors.preprocessor_directive =
    parsed_config["cpp_token_colors"]["preprocessor_directive"]
      .value_or<std::string>("#ffffff");
  _config.cpp_token_colors.identifier =
    parsed_config["cpp_token_colors"]["identifier"].value_or<std::string>(
      "#ffffff");
  _config.cpp_token_colors.number =
    parsed_config["cpp_token_colors"]["number"].value_or<std::string>(
      "#ffffff");
  _config.cpp_token_colors.function =
    parsed_config["cpp_token_colors"]["function"].value_or<std::string>(
      "#ffffff");
  _config.cpp_token_colors.header =
    parsed_config["cpp_token_colors"]["header"].value_or<std::string>(
      "#ffffff");

  _config.cursor.color =
    parsed_config["cursor"]["color"].value_or<std::string>("#ffffff");
  _config.cursor.style =
    parsed_config["cursor"]["style"].value_or<std::string>("ibeam");
  _config.cursor.ibeam_width =
    parsed_config["cursor"]["ibeam_width"].value_or<uint8>(2);

  return true;
}

const config& ConfigManager::get_config_struct() const noexcept
{
  return _config;
}