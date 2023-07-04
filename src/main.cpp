#include <fstream>
#include <string>
#include <vector>
#include "../include/cairo_context.hpp"
#include "../include/macros.hpp"
#include "../include/rocket_render.hpp"
#include "../include/sdl2.hpp"
#include "../include/window.hpp"

/// @brief Reads file contents as lines.
/// @param file_path path to file.
/// @return Returns pointer to vector of strings,
///         this memory should be freed by the user.
std::vector<std::string>* read_file(const std::string& file_path);

int main(int argc, char** argv)
{
  // Checking arguments
  if(argc < 2)
  {
    FATAL_BOII("Error: No file is provided as argument.");
    INFO_BOII("Usage: text-editor-software-rendering.exe <file_path>");
    exit(1);
  }

  // Loading file contents
  std::string file_path(argv[1]);
  std::vector<std::string>* contents = read_file(file_path);
  if(!contents)
  {
    FATAL_BOII("Unable to load file: %s", argv[1]);
    exit(1);
  }

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
  CairoContext::get_instance()->set_context_font("JetBrainsMono", 16);

  // main loop
  bool redraw = true;
  float32 scroll_y_offset = 0;
  while(1)
  {
    SDL_Event event;
    if(SDL_WaitEvent(&event))
    {
      if(event.type == SDL_QUIT)
      {
        delete contents;
        CairoContext::delete_instance();
        delete window;
        SDL_Quit();
        return 0;
      }
      if(event.type == SDL_WINDOWEVENT)
      {
        if(event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
          window->reload_window_surface();
          CairoContext::get_instance()->reload_context(*window);
          CairoContext::get_instance()->set_context_font("JetBrainsMono", 16);
          redraw = true;
        }
      }
      if(event.type == SDL_MOUSEWHEEL)
      {
        scroll_y_offset += event.wheel.preciseY;
        redraw = true;
      }
    }

    if(!redraw)
    {
      continue;
    }

    // Rendering
    // double xc = 220.0;
    // double yc = 240.0;
    // double radius = 200.0;
    // double angle1 = 45.0 * (M_PI / 180.0);
    // double angle2 = 180.0 * (M_PI / 180.0);

    // cairo_t* cr = CairoContext::get_instance()->get_context();
    // cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
    // cairo_set_line_width(cr, 10.0);
    // cairo_arc(cr, xc, yc, radius, angle1, angle2);
    // cairo_stroke(cr);

    // cairo_set_source_rgba(cr, 1, 0.2, 0.2, 0.6);
    // cairo_set_line_width(cr, 6.0);

    // cairo_arc(cr, xc, yc, 10.0, 0, 2 * M_PI);
    // cairo_fill(cr);

    // cairo_arc(cr, xc, yc, radius, angle1, angle1);
    // cairo_line_to(cr, xc, yc);
    // cairo_arc(cr, xc, yc, radius, angle2, angle2);
    // cairo_line_to(cr, xc, yc);
    // cairo_stroke(cr);

    // cairo_close_path(cr);

    // Rendering text
    // RocketRender::text(50, 50, "Hola!", {0, 0, 0, 255});

    window->clear_with_color({255, 255, 255, 255});

    float32 y = scroll_y_offset;
    for(const std::string& line : *contents)
    {
      // cairo_text_extents_t text_extents;
      // cairo_text_extents(
      // CairoContext::get_instance()->get_context(), line.c_str(), &text_extents);
      // if(y < 0 && -y > text_extents.height)
      // {
      //   y += text_extents.height;
      //   continue;
      // }
      cairo_font_extents_t font_extents;
      cairo_font_extents(CairoContext::get_instance()->get_context(), &font_extents);
      if(y < 0 && -y > font_extents.height)
      {
        y += font_extents.height;
        continue;
      }
      RocketRender::text(0, y, line, {0, 0, 0, 255});
      y += font_extents.height;
      if(y > window->height())
      {
        break;
      }
    }

    window->update();

    redraw = false;
  }

  delete contents;
  CairoContext::delete_instance();
  delete window;
  SDL_Quit();

  return 0;
}

std::vector<std::string>* read_file(const std::string& file_path)
{
  std::ifstream file(file_path);
  if(file.is_open())
  {
    auto* contents = new std::vector<std::string>();
    while(file.good())
    {
      contents->push_back("");
      std::getline(file, contents->back());
    }
    return contents;
  }
  log_error("Unable to open file: %s", file_path.c_str());
  return nullptr;
}
