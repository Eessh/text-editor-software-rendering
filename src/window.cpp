#include "../include/window.hpp"
#include "../include/macros.hpp"
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#  include "../SDL2-2.26.5/x86_64-w64-mingw32/include/SDL2/SDL_syswm.h"
#endif

Window::Window(const std::string& title,
               const uint16 width,
               const uint16 height)
  : _title(title)
  , _width(width)
  , _height(height)
  , _window(nullptr)
  , _window_surface(nullptr)
  , _cairo_context(nullptr)
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

  cairo_surface_t* cairo_surface =
    cairo_image_surface_create_for_data((unsigned char*)_window_surface->pixels,
                                        CAIRO_FORMAT_RGB24,
                                        _window_surface->w,
                                        _window_surface->h,
                                        _window_surface->pitch);
  if(!cairo_surface)
  {
    FATAL_BOII("Unable to create cairo surface for window!");
    SDL_DestroyWindow(_window);
    SDL_Quit();
    exit(1);
  }
  cairo_surface_set_device_scale(cairo_surface, 1, 1);

  _cairo_context = cairo_create(cairo_surface);
  if(!_cairo_context)
  {
    FATAL_BOII("Unable to create cairo context for window!");
    cairo_surface_destroy(cairo_surface);
    SDL_DestroyWindow(_window);
    SDL_Quit();
    exit(1);
  }
  cairo_surface_destroy(cairo_surface);
}

Window::~Window()
{
  cairo_destroy(_cairo_context);
  SDL_DestroyWindow(_window);
}

const uint16& Window::width() const
{
  return _width;
}

const uint16& Window::height() const
{
  return _height;
}

const std::string& Window::title() const
{
  return _title;
}

uint16& Window::width()
{
  return _width;
}

uint16& Window::height()
{
  return _height;
}

std::string& Window::title()
{
  return _title;
}

SDL_Surface* Window::surface()
{
  return _window_surface;
}

bool Window::set_icon(const char* icon_path)
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

bool Window::set_dark_theme()
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

cairo_t* Window::cairo_context()
{
  return _cairo_context;
}

void Window::reload_cairo_context()
{
  _window_surface = SDL_GetWindowSurface(_window);
  if(!_window_surface)
  {
    FATAL_BOII("Unable to reload window surface: %s", SDL_GetError());
  }

  cairo_destroy(_cairo_context);
  cairo_surface_t* cairo_surface =
    cairo_image_surface_create_for_data((unsigned char*)_window_surface->pixels,
                                        CAIRO_FORMAT_RGB24,
                                        _window_surface->w,
                                        _window_surface->h,
                                        _window_surface->pitch);
  cairo_surface_set_device_scale(cairo_surface, 1, 1);

  _cairo_context = cairo_create(cairo_surface);
  cairo_surface_destroy(cairo_surface);
}