#pragma once

#include <string>
#include "types.hpp"

typedef struct config
{
  struct window
  {
    uint16 width, height;
  } window;

  struct colorscheme
  {
    std::string bg, fg, red, orange, yellow, green, cyan, blue, purple, white,
      black, gray, highlight, comment;
  } colorscheme;
} config;

class ConfigManager
{
public:
  ConfigManager(const ConfigManager& manager) = delete;
  ConfigManager(ConfigManager&& manager) = delete;
  ConfigManager operator=(const ConfigManager& manager) = delete;
  ConfigManager operator=(ConfigManager&& manager) = delete;

  ~ConfigManager() noexcept = default;

  static void create_instance() noexcept;

  [[nodiscard]] static ConfigManager* get_instance() noexcept;

  static void delete_instance() noexcept;

  [[nodiscard]] bool
  load_config(const std::string& config_file_path = "") noexcept;

  [[nodiscard]] const config& get_config_struct() const noexcept;

private:
  config _config;

  ConfigManager() noexcept = default;

  static ConfigManager* _instance;
};