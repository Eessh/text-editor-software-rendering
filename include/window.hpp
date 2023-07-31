#pragma once

#include <string>
#include "cairo.hpp"
#include "sdl2.hpp"
#include "types.hpp"

class Window
{
public:
  /// @brief Creates a window.
  /// @param title title of window.
  /// @param width width of window.
  /// @param height height of window.
  /// @throws No exceptions.
  Window(const std::string& title,
         const uint16 width,
         const uint16 height) noexcept;

  Window(const Window& window) = delete;
  Window(Window&& window) = delete;

  /// @brief Window destructor.
  /// @throws No exceptions.
  ~Window() noexcept;

  /// @brief Window width.
  /// @return Returns const reference to window width.
  /// @throws No exceptions.
  const uint16& width() const noexcept;

  /// @brief Window height.
  /// @return Returns const reference to window height.
  /// @throws No exceptions.
  const uint16& height() const noexcept;

  /// @brief Window title.
  /// @return Returns const reference to window title.
  /// @throws No exceptions.
  const std::string& title() const noexcept;

  /// @brief Set window width.
  /// @return Returns reference to window width.
  /// @throws No exceptions.
  uint16& width() noexcept;

  /// @brief Set window height.
  /// @return Returns reference to window height.
  /// @throws No exceptions.
  uint16& height() noexcept;

  /// @brief Set window title.
  /// @return Returns reference to window title.
  /// @throws No exceptions.
  std::string& title() noexcept;

  /// @brief Window's surface.
  /// @return Returns pointer to window's surface.
  /// @throws No exceptions.
  SDL_Surface* surface() const noexcept;

  /// @brief Sets window icon in windows and linux, icon should be BMP format.
  /// @param icon_path icon file path.
  /// @return returns false if icon cannot be found at given path.
  /// @throws No exceptions.
  bool set_icon(const char* icon_path) const noexcept;

  /// @brief Sets dark mode to window.
  /// @return returns false if cannot be set (on platforms linux, macos).
  /// @throws No exceptions.
  bool set_dark_theme() const noexcept;

  /// @brief Handles reloading of window surface, when window is resized.
  ///        This should be called when window resize event occurs.
  /// @param event const reference to the event.
  /// @throws No exceptions.
  void handle_resize(const SDL_Event& event) noexcept;

  /// @brief Handles reloading of window surface, when window is maxmized.
  ///        This should be called when window maximize event occurs.
  /// @param event const reference to the event.
  /// @throws No exceptions.
  void handle_maximize(const SDL_Event& event) noexcept;

  /// @brief Reloads the cairo context of window,
  ///        this should be called when window is resized.
  /// @throws No exceptions.
  void reload_window_surface() noexcept;

  /// @brief Clears the whole window surface with given color.
  /// @param color color with which window should be painted.
  /// @throws No exceptions.
  void clear_with_color(const SDL_Color& color) const noexcept;

  /// @brief Toggles fullscreen mode for window.
  /// @throws No exceptions.
  void toggle_fullscreen() noexcept;

  /// @brief Updates given rectangle portions in window.
  /// @param rects pointer to SDL_Rect array.
  /// @param rects_count length of the SDL_Rect array.
  /// @throws No exceptions.
  void update_rects(SDL_Rect* rects, int rects_count) const noexcept;

  /// @brief Updates window surface (i.e swaps the updated window surface).
  /// @throws No exceptions.
  void update() const noexcept;

private:
  /// @brief window width
  uint16 _width;

  /// @brief window height
  uint16 _height;

  /// @brief window title
  std::string _title;

  /// @brief window in fullscreen mode
  bool _fullscreen;

  /// @brief pointer to SDL_Window
  SDL_Window* _window;

  /// @brief pointer to window's surface
  SDL_Surface* _window_surface;

  friend class CairoContext;
};