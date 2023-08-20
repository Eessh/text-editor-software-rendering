#include "../include/window.hpp"
#include "../include/macros.hpp"
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#  include "../SDL2-2.26.5/x86_64-w64-mingw32/include/SDL2/SDL_syswm.h"
#endif

Window::Window(const std::string& title,
               const uint16 width,
               const uint16 height) noexcept
  : _title(title)
  , _width(width)
  , _height(height)
  , _fullscreen(false)
  , _window(nullptr)
  , _window_surface(nullptr)
{
  _window = SDL_CreateWindow(_title.c_str(),
                             SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED,
                             _width,
                             _height,
                             SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
                               SDL_WINDOW_ALLOW_HIGHDPI);
  if(!_window)
  {
    FATAL_BOII("Unable to create window: %s", SDL_GetError());
    SDL_Quit();
    exit(1);
  }

  _window_surface = SDL_GetWindowSurface(_window);
  if(!_window_surface)
  {
    FATAL_BOII("Unable to get window surface: %s", SDL_GetError());
    SDL_DestroyWindow(_window);
    SDL_Quit();
    exit(1);
  }

  // White background
  SDL_FillRect(
    _window_surface, NULL, SDL_MapRGB(_window_surface->format, 255, 255, 255));
}

Window::~Window() noexcept
{
  SDL_DestroyWindow(_window);
}

const uint16& Window::width() const noexcept
{
  return _width;
}

const uint16& Window::height() const noexcept
{
  return _height;
}

const std::string& Window::title() const noexcept
{
  return _title;
}

uint16& Window::width() noexcept
{
  return _width;
}

uint16& Window::height() noexcept
{
  return _height;
}

std::string& Window::title() noexcept
{
  return _title;
}

SDL_Surface* Window::surface() const noexcept
{
  return _window_surface;
}

bool Window::set_icon(const char* icon_path) const noexcept
{
  SDL_Surface* rocket_icon = SDL_LoadBMP(icon_path);
  if(!rocket_icon)
  {
    ERROR_BOII("Unable to load rocket.bmp icon!");
    return false;
  }
  SDL_SetWindowIcon(_window, rocket_icon);
  SDL_FreeSurface(rocket_icon);
  return true;
}

bool Window::set_dark_theme() const noexcept
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
  SDL_SysWMinfo wmi;
  SDL_VERSION(&wmi.version);
  SDL_GetWindowWMInfo(_window, &wmi);
  HWND hwnd = wmi.info.win.window;

  HMODULE uxtheme = LoadLibraryA("uxtheme.dll");
  HMODULE dwm = LoadLibraryA("dwmapi.dll");

  if(uxtheme && dwm)
  {
    typedef HRESULT (*SetWindowThemePTR)(HWND, const wchar_t*, const wchar_t*);
    SetWindowThemePTR SetWindowTheme =
      (SetWindowThemePTR)GetProcAddress(uxtheme, "SetWindowTheme");

    typedef HRESULT (*DwmSetWindowAttributePTR)(HWND, DWORD, LPCVOID, DWORD);
    DwmSetWindowAttributePTR DwmSetWindowAttribute =
      (DwmSetWindowAttributePTR)GetProcAddress(dwm, "DwmSetWindowAttribute");

    if(SetWindowTheme && DwmSetWindowAttribute)
    {
      SetWindowTheme(hwnd, L"DarkMode_Explorer", NULL);

      BOOL darkMode = 1;
      if(!DwmSetWindowAttribute(hwnd, 20, &darkMode, sizeof darkMode))
      {
        DwmSetWindowAttribute(hwnd, 19, &darkMode, sizeof darkMode);
      }
      return true;
    }
    return true;
  }
  return true;
#endif
  return false;
}

void Window::handle_resize(const SDL_Event& event) noexcept
{
  _width = static_cast<uint16>(event.window.data1);
  _height = static_cast<uint16>(event.window.data2);
  this->reload_window_surface();
}

void Window::handle_maximize(const SDL_Event& event) noexcept
{
  int w, h;
  SDL_GetWindowSize(_window, &w, &h);
  _width = static_cast<uint16>(w);
  _height = static_cast<uint16>(h);
  this->reload_window_surface();
}

void Window::clear_with_color(const SDL_Color& color) const noexcept
{
  SDL_FillRect(_window_surface,
               NULL,
               SDL_MapRGB(_window_surface->format, color.r, color.g, color.b));
}

void Window::toggle_fullscreen() noexcept
{
  if(_fullscreen)
  {
    SDL_SetWindowFullscreen(_window, 0);
    _fullscreen = false;
  }
  else
  {
    SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    _fullscreen = true;
  }

  // syncing window's size
  int w, h;
  SDL_GetWindowSize(_window, &w, &h);
  _width = static_cast<uint16>(w);
  _height = static_cast<uint16>(h);
}

void Window::reload_window_surface() noexcept
{
  _window_surface = SDL_GetWindowSurface(_window);
  if(!_window_surface)
  {
    FATAL_BOII("Unable to reload window surface: %s", SDL_GetError());
  }
}

void Window::update_rects(SDL_Rect* rects, int rects_count) const noexcept
{
  SDL_UpdateWindowSurfaceRects(_window, rects, rects_count);
}

void Window::update() const noexcept
{
  SDL_UpdateWindowSurface(_window);
}
