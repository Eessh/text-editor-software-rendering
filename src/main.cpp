#include <fstream>
#include <string>
#include <vector>
#include "../include/buffer.hpp"
#include "../include/cairo_context.hpp"
#include "../include/macros.hpp"
#include "../include/rocket_render.hpp"
#include "../include/sdl2.hpp"
#include "../include/window.hpp"

bool animator(float32* animatable, const float32* target);

int main(int argc, char** argv)
{
  // Checking arguments
  if(argc < 2)
  {
    FATAL_BOII("Error: No file is provided as argument.");
    INFO_BOII("Usage: text-editor-software-rendering.exe <file_path>");
    exit(1);
  }

  // Creating buffer
  std::string file_path(argv[1]);
  Buffer buffer;
  if(!buffer.load_from_file(file_path))
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

  // Font extents
  cairo_font_extents_t font_extents;
  cairo_font_extents(CairoContext::get_instance()->get_context(),
                     &font_extents);

  // Initial render
  {
    window->clear_with_color({255, 255, 255, 255});
    float32 y = 0.0f;
    for(const std::string& line : buffer.lines())
    {
      if(y < 0 && -y > font_extents.height)
      {
        y += font_extents.height;
        continue;
      }
      RocketRender::text(0, y, line, {0, 0, 0, 255});
      y += font_extents.height;
      if(y > static_cast<int16>(window->height()))
      {
        break;
      }
    }
    window->update();
  }

  // main loop
  bool redraw = false, fingerdown = false;
  float32 scroll_y_offset = 0.0f, scroll_y_target = 0.0f;
  uint8 scroll_sensitivity = 80, wait_time = 250;
  while(1)
  {
    double frame_start_time =
      SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
    SDL_Event event;
    if(SDL_PollEvent(&event))
    {
      if(event.type == SDL_QUIT)
      {
        CairoContext::delete_instance();
        delete window;
        SDL_Quit();
        return 0;
      }
      else if(event.type == SDL_WINDOWEVENT)
      {
        if(event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
          window->handle_resize(event);
          CairoContext::get_instance()->reload_context(*window);
          CairoContext::get_instance()->set_context_font("JetBrainsMono", 16);
          redraw = true;
        }
        // these should be handled in linux
        else if(event.window.event == SDL_WINDOWEVENT_EXPOSED ||
                event.window.event == SDL_WINDOWEVENT_SHOWN)
        {
          redraw = true;
        }
      }
      else if(event.type == SDL_MOUSEWHEEL)
      {
        scroll_y_target +=
          static_cast<float32>(scroll_sensitivity) * event.wheel.preciseY;
        if(scroll_y_target > 0)
        {
          scroll_y_target = 0.0f;
        }
        if(scroll_y_target <
           -static_cast<float32>(buffer.length()) * font_extents.height)
        {
          scroll_y_target =
            -static_cast<float32>(buffer.length()) * font_extents.height;
        }
      }
      else if(event.type == SDL_KEYDOWN)
      {
        if(event.key.keysym.sym == SDLK_LEFT)
        {
          buffer.execute_cursor_command(BufferCursorCommand::MOVE_LEFT);
          redraw = true;
        }
        else if(event.key.keysym.sym == SDLK_RIGHT)
        {
          buffer.execute_cursor_command(BufferCursorCommand::MOVE_RIGHT);
          redraw = true;
        }
        else if(event.key.keysym.sym == SDLK_UP)
        {
          buffer.execute_cursor_command(BufferCursorCommand::MOVE_UP);
          redraw = true;
        }
        else if(event.key.keysym.sym == SDLK_DOWN)
        {
          buffer.execute_cursor_command(BufferCursorCommand::MOVE_DOWN);
          redraw = true;
        }
      }
    }

    redraw = redraw || animator(&scroll_y_offset, &scroll_y_target);

    if(redraw)
    {
      window->clear_with_color({255, 255, 255, 255});

      // visualizing extents
      // const std::string text = "#include <fstream>";
      // cairo_text_extents_t text_extents;
      // cairo_text_extents(
      //   CairoContext::get_instance()->get_context(), text.c_str(), &text_extents);

      // RocketRender::rectangle_outlined(0,
      //                                  0,
      //                                  text_extents.x_advance -
      //                                    text_extents.x_bearing,
      //                                  font_extents.height,
      //                                  {0, 0, 0, 255});
      
      // // const char* text2 = "`1234567890-=~!@#$%^&*()_+";
      // const std::string text2 = "abcdef";
      // cairo_text_extents_t text_extents2;
      // cairo_text_extents(
      //   CairoContext::get_instance()->get_context(), text2.c_str(), &text_extents2);

      // RocketRender::rectangle_outlined(0,
      //                                  font_extents.height+10,
      //                                  text_extents2.x_advance -
      //                                    text_extents2.x_bearing,
      //                                  font_extents.height,
      //                                  {0, 0, 0, 255});

      // DEBUG_BOII("font metrics: ascent = %lf, descent = %lf, height = %lf",
      //            font_extents.ascent,
      //            font_extents.descent,
      //            font_extents.height);
      // DEBUG_BOII("text metrics: height = %lf, width = %lf, x_advance = %lf, "
      //            "x_bearing = %lf, y_advance = %lf, y_bearing = %lf",
      //            text_extents.height,
      //            text_extents.width,
      //            text_extents.x_advance,
      //            text_extents.x_bearing,
      //            text_extents.y_advance,
      //            text_extents.y_bearing);
      // // float32 pen_x = 0.0f;
      // // for (const char& c: text) {
      // //   char str[2] = {c, '\0'};
      // //   cairo_text_extents_t extents;
      // //   cairo_t* cr = CairoContext::get_instance()->get_context();
      // //   cairo_text_extents(cr, str, &extents);
      // //   cairo_move_to(cr, pen_x+extents.x_bearing, font_extents.height-font_extents.descent);
      // //   cairo_show_text(cr, str);
      // //   pen_x += extents.x_advance;
      // // }
      // cairo_move_to(CairoContext::get_instance()->get_context(),
      //               -text_extents.x_bearing,
      //               font_extents.height-font_extents.descent);
      // cairo_show_text(CairoContext::get_instance()->get_context(), text.c_str());

      // cairo_move_to(CairoContext::get_instance()->get_context(),
      //               -text_extents2.x_bearing,
      //               font_extents.height+10+font_extents.ascent-text_extents2.height+font_extents.descent/2-text_extents2.y_bearing);
      // cairo_show_text(CairoContext::get_instance()->get_context(), text2.c_str());
      // RocketRender::text(-text_extents.x_bearing,
      //                    text_extents.height + text_extents.y_bearing,
      //                    "q-g-f-i-W",
      //                    {0, 0, 0, 255});

      int32 y = scroll_y_offset;
      for(const std::string& line : buffer.lines())
      {
        // cairo_text_extents_t text_extents;
        // cairo_text_extents(
        // CairoContext::get_instance()->get_context(), line.c_str(), &text_extents);
        // if(y < 0 && -y > text_extents.height)
        // {
        //   y += text_extents.height;
        //   continue;
        // }
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

      // drawing scrollbar
      if(buffer.length() * font_extents.height > window->height())
      {
        float32 content_height =
          buffer.length() * font_extents.height + window->height();
        float32 scrollbar_edge_padding = 2.0f;
        float32 viewport_height = window->height() - 2 * scrollbar_edge_padding;
        float32 ratio = viewport_height / content_height;
        float32 scrollbar_min_height = 18.0f;
        float32 scrollbar_width = 8.0f;
        float32 scrollbar_height = ratio * viewport_height;
        if(scrollbar_height < scrollbar_min_height)
        {
          // re-calculating scrollbar height with adjusted metrics
          viewport_height -= scrollbar_min_height - scrollbar_height;
          ratio = viewport_height / content_height;
          scrollbar_height = scrollbar_min_height;
        }
        RocketRender::rectangle_rounded(
          window->width() - scrollbar_width - scrollbar_edge_padding,
          -scroll_y_offset * ratio + scrollbar_edge_padding,
          scrollbar_width,
          scrollbar_height,
          scrollbar_width / 2,
          {128, 64, 64, 128});
      }

      // drawring cursor
      std::pair<uint32, int32> cursor_coords = buffer.cursor_coords();
      RocketRender::rectangle_filled(
        font_extents.max_x_advance * (cursor_coords.second + 1),
        scroll_y_offset + font_extents.height * cursor_coords.first,
        2,
        font_extents.height,
        {0, 0, 0, 255});

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

  CairoContext::delete_instance();
  delete window;
  SDL_Quit();

  return 0;
}

bool animator(float32* animatable, const float32* target)
{
  const float32 delta = *target - *animatable;
  if(delta == 0)
  {
    return false;
  }

  if(abs(delta) < 0.1f)
  {
    *animatable = *animatable + delta;
    return true;
  }

  *animatable += delta * 0.4f;
  return true;
}
