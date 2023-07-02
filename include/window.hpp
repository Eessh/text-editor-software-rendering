#pragma once

#include <string>
#include "cairo.hpp"
#include "sdl2.hpp"
#include "types.hpp"

class Window
{
public:
  /// @brief Creates a window.
  /// @param title itle of window.
  /// @param width width of window.
  /// @param height height of window.
  Window(const std::string& title, const uint16 width, const uint16 height);

  Window(const Window& window) = delete;
  Window(Window&& window) = delete;

  ~Window();

  /// @brief Window width.
  /// @return Returns const reference to window width.
  const uint16& width() const;

  /// @brief Window height.
  /// @return Returns const reference to window height.
  const uint16& height() const;

  /// @brief Window title.
  /// @return Returns const reference to window title.
  const std::string& title() const;

  /// @brief Set window width.
  /// @return Returns reference to window width.
  uint16& width();

  /// @brief Set window height.
  /// @return Returns reference to window height.
  uint16& height();

  /// @brief Set window title.
  /// @return Returns reference to window title.
  std::string& title();

  /// @brief Window's surface.
  /// @return Returns pointer to window's surface.
  SDL_Surface* surface();

  /// @brief Sets window icon in windows and linux, icon should be BMP format.
  /// @param icon_path icon file path.
  /// @return returns false if icon cannot be found at given path.
  bool set_icon(const char* icon_path);

  /// @brief Sets dark mode to window.
  /// @return returns false if cannot be set (on platforms linux, macos).
  bool set_dark_theme();

  /// @brief Reloads the cairo context of window,
  ///        this should be called when window is resized.
  void reload_window_surface();

private:
  /// @brief window width
  uint16 _width;

  /// @brief window height
  uint16 _height;

  /// @brief window title
  std::string _title;

  /// @brief pointer to SDL_Window
  SDL_Window* _window;

  /// @brief pointer to window's surface
  SDL_Surface* _window_surface;

  friend class CairoContext;
};