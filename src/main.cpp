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
  uint8 wait_time = 250;
  while(1)
  {
    double frame_start_time =
      SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
    SDL_Event event;
    if(SDL_PollEvent(&event))
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

    if(redraw)
    {
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
        cairo_font_extents(CairoContext::get_instance()->get_context(),
                           &font_extents);
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
    else
    {
      SDL_WaitEventTimeout(NULL, wait_time);
    }

    double frame_end_time =
      SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
    double time_elapsed = frame_end_time - frame_start_time;
    INFO_BOII("Frame time: %lf", time_elapsed);
    SDL_Delay((1 / (60 - time_elapsed)) * 1000);
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
