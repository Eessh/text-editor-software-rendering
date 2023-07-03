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

  // Loading font
  CairoContext::get_instance()->load_font(
    "JetBrainsMono",
    "assets/fonts/JetBrains Mono Regular Nerd Font Complete.ttf");
  CairoContext::get_instance()->set_context_font("JetBrainsMono", 18);

  // main loop
  bool redraw = true;
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
        if(event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
          window->reload_window_surface();
          CairoContext::get_instance()->reload_context(*window);
          CairoContext::get_instance()->set_context_font("JetBrainsMono", 18);
          redraw = true;
        }
      }
    }

    if(!redraw)
    {
      continue;
    }

    // Rendering
    double xc = 220.0;
    double yc = 240.0;
    double radius = 200.0;
    double angle1 = 45.0 * (M_PI / 180.0);
    double angle2 = 180.0 * (M_PI / 180.0);

    cairo_t* cr = CairoContext::get_instance()->get_context();
    cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
    cairo_set_line_width(cr, 10.0);
    cairo_arc(cr, xc, yc, radius, angle1, angle2);
    cairo_stroke(cr);

    cairo_set_source_rgba(cr, 1, 0.2, 0.2, 0.6);
    cairo_set_line_width(cr, 6.0);

    cairo_arc(cr, xc, yc, 10.0, 0, 2 * M_PI);
    cairo_fill(cr);

    cairo_arc(cr, xc, yc, radius, angle1, angle1);
    cairo_line_to(cr, xc, yc);
    cairo_arc(cr, xc, yc, radius, angle2, angle2);
    cairo_line_to(cr, xc, yc);
    cairo_stroke(cr);

    cairo_close_path(cr);

    // Rendering text
    const char text[] = "Hola!";
    // cairo_text_extents_t text_extents;
    // cairo_text_extents(cr, text, &text_extents);
    cairo_move_to(cr, 500, 500);
    // cairo_move_to(cr, 50, 50 + text_extents.height);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_show_text(cr, text);

    window->update();

    redraw = false;
  }

  CairoContext::delete_instance();
  delete window;
  SDL_Quit();

  return 0;
}
