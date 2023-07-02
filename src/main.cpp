#include "../include/macros.hpp"
#include "../include/sdl2.hpp"
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#  include "../SDL2-2.26.5/x86_64-w64-mingw32/include/SDL2/SDL_syswm.h"
#endif

/// @brief Sets window icons in windows and linux.
/// @param window Pointer to SDL_Window.
/// @param icon_path Icon file path.
void set_window_icon(SDL_Window* window, const char* icon_path);

/// @brief Sets dark theme in windows.
/// @param window Pointer to SDL_Window.
void set_dark_theme(SDL_Window* window);

int main(int argc, char** argv)
{
  // Initializing SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
  {
    FATAL_BOII("Unable to initialize SDL: %s", SDL_GetError());
    exit(1);
  }

  // Getting current display mode dimensions
  SDL_DisplayMode display_mode;
  SDL_GetCurrentDisplayMode(0, &display_mode);

  // Creating window
  SDL_Window* window = SDL_CreateWindow(
    "Text Editor - Software Rendering",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    0.8 * display_mode.w,
    0.8 * display_mode.h,
    SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  if(!window)
  {
    FATAL_BOII("Unable to create window: %s", SDL_GetError());
    SDL_Quit();
    exit(1);
  }

  // Setting window icon
  set_window_icon(window, "assets/images/rocket.bmp");

  // Setting dark theme
  set_dark_theme(window);

  // main loop
  while(1)
  {
    SDL_Event event;
    if(SDL_WaitEvent(&event))
    {
      if(event.type == SDL_QUIT)
      {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
      }
    }
  }

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

void set_window_icon(SDL_Window* window, const char* icon_path)
{
  SDL_Surface* rocket_icon = SDL_LoadBMP("assets/images/rocket.bmp");
  if(!rocket_icon)
  {
    log_error("Unable to load rocket.bmp icon!");
  }
  SDL_SetWindowIcon(window, rocket_icon);
  SDL_FreeSurface(rocket_icon);
}

void set_dark_theme(SDL_Window* window)
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
  SDL_SysWMinfo wmi;
  SDL_VERSION(&wmi.version);
  SDL_GetWindowWMInfo(window, &wmi);
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
    }
  }
#endif
}