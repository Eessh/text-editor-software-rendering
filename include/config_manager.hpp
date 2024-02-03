#pragma once

#include <filesystem>
#include "config.hpp"

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

  [[nodiscard]] bool reload_config_if_changed() noexcept;

  [[nodiscard]] const config& get_config_struct() const noexcept;

private:
  config _config;

  std::string _config_path;

  std::filesystem::file_time_type _last_config_write_time;

  ConfigManager() noexcept = default;

  static ConfigManager* _instance;
};