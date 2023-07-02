#include "../include/cairo_context.hpp"
#include "../include/macros.hpp"
#include "../include/sdl2.hpp"
#include "../include/window.hpp"

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
  Window* window = new Window("Text Editor - Software Rendering",
                              0.8 * display_mode.w,
                              0.8 * display_mode.h);
  window->set_icon("assets/images/rocket.bmp");
  window->set_dark_theme();

  // Creating cairo context
  CairoContext::create_instance();
  CairoContext::get_instance()->initialize(*window);

  // main loop
  while(1)
  {
    SDL_Event event;
    if(SDL_WaitEvent(&event))
    {
      if(event.type == SDL_QUIT)
      {
        SDL_Quit();
        return 0;
      }
      if(event.type == SDL_WINDOWEVENT)
      {
        if(event.window.type == SDL_WINDOWEVENT_RESIZED)
        {
          window->reload_window_surface();
          CairoContext::get_instance()->reload_context(*window);
        }
      }
    }
  }

  CairoContext::delete_instance();
  delete window;
  SDL_Quit();

  return 0;
}
