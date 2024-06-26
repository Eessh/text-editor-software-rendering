#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include "../cpp-tokenizer/cpp_tokenizer.hpp"
#include "../include/buffer.hpp"
#include "../include/cairo_context.hpp"
#include "../include/config_manager.hpp"
#include "../include/cpp_tokenizer_cache.hpp"
#include "../include/cursor_manager.hpp"
#include "../include/incremental_render_update.hpp"
#include "../include/macros.hpp"
#include "../include/rocket_render.hpp"
#include "../include/sdl2.hpp"
#include "../include/utils.hpp"
#include "../include/window.hpp"

int main(int argc, char** argv)
{
  // Checking arguments
  //  if(argc < 2)
  //  {
  //    FATAL_BOII("Error: No file is provided as argument.");
  //    INFO_BOII("Usage: text-editor-software-rendering.exe <file_path>");
  //    exit(1);
  //  }

  // Creating config manager
  ConfigManager::create_instance();
  if(!ConfigManager::get_instance()->load_config())
  {
    FATAL_BOII("Unable to load config: config.toml!");
    exit(1);
  }

  // Creating buffer
  //  std::string file_path(argv[1]);
  Buffer buffer;
  if(argc > 1 && !buffer.load_from_file(argv[1]))
  {
    FATAL_BOII("Unable to load file: %s", argv[1]);
    exit(1);
  }

  // Creating tokenizer cache
  CppTokenizerCache tokenizer_cache;
  tokenizer_cache.build_cache(buffer);

  // Initializing SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
  {
    FATAL_BOII("Unable to initialize SDL: %s", SDL_GetError());
    exit(1);
  }

  // Setting hints
  /// bypassing compositors in X11
  SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
  SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
  SDL_SetHint("SDL_MOUSE_DOUBLE_CLICK_RADIUS", "4");

  // Enabling screen saver
  SDL_EnableScreenSaver();

  Window* window = new Window(
    "Rocket" +
      (argc > 1
         ? std::filesystem::absolute(std::filesystem::path(argv[1])).string()
         : ""),
    ConfigManager::get_instance()->get_config_struct().window.width,
    ConfigManager::get_instance()->get_config_struct().window.height);
  {
    bool _ = window->set_icon("assets/images/rocket.bmp");
    _ = window->set_dark_theme();
  }

  // Creating cairo context
  CairoContext::create_instance();
  CairoContext::get_instance()->initialize(*window);

  // Loading font
  if(!CairoContext::get_instance()->load_font(
       "code_font",
       ConfigManager::get_instance()->get_config_struct().code_font))
  {
    ERROR_BOII("Unable to load font: JetBrainsMono!");
  }
  CairoContext::get_instance()->set_context_font(
    "code_font", ConfigManager::get_instance()->get_config_struct().font_size);

  // Font extents
  cairo_font_extents_t font_extents =
    CairoContext::get_instance()->get_font_extents();

  // Creating cursor manager and loading system cursors
  CursorManager::create_instance();
  CursorManager::get_instance()->load_system_cursors();
  CursorManager::get_instance()->set_ibeam();

  // Creating tokenizer
  // CppTokenizer::Tokenizer tokenizer;

  // main loop
  float32 scroll_y_offset = 0.0f, scroll_y_target = 0.0f;
  uint8 scroll_sensitivity = ConfigManager::get_instance()
                               ->get_config_struct()
                               .scrolling.sensitivity,
        wait_time = 250;
  bool redraw = true, mouse_single_tap_down = false,
       mouse_double_tap_down = false, mouse_triple_tap_down = false;
  SDL_StartTextInput();
  while(true)
  {
    double frame_start_time =
      SDL_GetPerformanceCounter() /
      static_cast<double>(SDL_GetPerformanceFrequency());
    SDL_Event event;
    if(SDL_PollEvent(&event))
    {
      if(event.type == SDL_QUIT)
      {
        goto cleanup;
      }
      else if(event.type == SDL_WINDOWEVENT)
      {
        if(event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
          window->handle_resize(event);
          CairoContext::get_instance()->reload_context(*window);
          redraw = true;
        }
        else if(event.window.event == SDL_WINDOWEVENT_MAXIMIZED)
        {
          window->handle_maximize(event);
          CairoContext::get_instance()->reload_context(*window);
          redraw = true;
        }
        // these should be handled in linux
        else if(event.window.event == SDL_WINDOWEVENT_EXPOSED ||
                event.window.event == SDL_WINDOWEVENT_SHOWN)
        {
          redraw = true;
        }
      }
      /// important fix for linux - X11 or Wayland
      /// too many mouse motion events (especially in KDE)
      /// this issue almost made me quit this project
      else if(event.type == SDL_MOUSEMOTION)
      {
        SDL_PumpEvents();
        SDL_Event future_event;
        while(
          SDL_PeepEvents(
            &future_event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION) >
          0)
        {
          event.motion.x = future_event.motion.x;
          event.motion.y = future_event.motion.y;
          event.motion.xrel += future_event.motion.xrel;
          event.motion.yrel += future_event.motion.yrel;
        }

        float32 line_numbers_width =
          (std::to_string(buffer.length()).length() + 2) *
          font_extents.max_x_advance;
        std::pair<uint32, int32> buffer_grid_coords =
          mouse_coords_to_buffer_coords(event.motion.x,
                                        event.motion.y,
                                        line_numbers_width,
                                        scroll_y_offset,
                                        buffer);
        if(mouse_single_tap_down)
        {
          buffer.set_cursor_row(buffer_grid_coords.first);
          buffer.set_cursor_column(buffer_grid_coords.second);
          buffer.set_selection_end_coordinate(buffer_grid_coords);
          //          redraw = true;
        }
        else if(mouse_double_tap_down)
        {
          /// TODO: Implement mouse selection for words.
        }
        else if(mouse_triple_tap_down)
        {
          buffer.extend_line_selection_to_line(buffer_grid_coords.first);
          redraw = true;
        }
      }
      else if(event.type == SDL_MOUSEWHEEL)
      {
        SDL_PumpEvents();
        SDL_Event future_event;
        while(
          SDL_PeepEvents(
            &future_event, 1, SDL_GETEVENT, SDL_MOUSEWHEEL, SDL_MOUSEWHEEL) > 0)
        {
          event.wheel.preciseX += future_event.wheel.preciseX;
          event.wheel.preciseY += future_event.wheel.preciseY;
        }

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
        // Save file event
        if(event.key.keysym.sym == SDLK_s &&
           (event.key.keysym.mod & KMOD_LCTRL))
        {
          bool _ = buffer.save();
        }
        else if(event.key.keysym.sym == SDLK_LEFT)
        {
          if((event.key.keysym.mod & KMOD_LCTRL) &&
             (event.key.keysym.mod & KMOD_LSHIFT))
          {
            buffer.execute_selection_command(
              BufferSelectionCommand::EXTEND_TO_PREVIOUS_WORD_START);
          }
          else if(event.key.keysym.mod & KMOD_LCTRL)
          {
            buffer.execute_cursor_command(
              BufferCursorCommand::MOVE_TO_PREVIOUS_WORD_START);
          }
          else if(event.key.keysym.mod & KMOD_LSHIFT)
          {
            buffer.execute_selection_command(BufferSelectionCommand::MOVE_LEFT);
          }
          else
          {
            buffer.execute_cursor_command(BufferCursorCommand::MOVE_LEFT);
          }
          std::pair<uint32, int32> cursor_coords = buffer.cursor_coords();
          float32 effective_cursor_y =
            scroll_y_offset +
            static_cast<int32>(cursor_coords.first) * font_extents.height;
          if(effective_cursor_y < 0 ||
             effective_cursor_y + font_extents.height > window->height())
          {
            // delete last inserted command
            buffer.remove_most_recent_view_update_command();
          }
          redraw = true;
        }
        else if(event.key.keysym.sym == SDLK_RIGHT)
        {
          if((event.key.keysym.mod & KMOD_LCTRL) &&
             (event.key.keysym.mod & KMOD_LSHIFT))
          {
            buffer.execute_selection_command(
              BufferSelectionCommand::EXTENT_TO_NEXT_WORD_END);
          }
          else if(event.key.keysym.mod & KMOD_LCTRL)
          {
            buffer.execute_cursor_command(
              BufferCursorCommand::MOVE_TO_NEXT_WORD_END);
          }
          else if(event.key.keysym.mod & KMOD_LSHIFT)
          {
            buffer.execute_selection_command(
              BufferSelectionCommand::MOVE_RIGHT);
          }
          else
          {
            buffer.execute_cursor_command(BufferCursorCommand::MOVE_RIGHT);
          }
          std::pair<uint32, int32> cursor_coords = buffer.cursor_coords();
          float32 effective_cursor_y =
            scroll_y_offset +
            static_cast<int32>(cursor_coords.first) * font_extents.height;
          if(effective_cursor_y < 0 ||
             effective_cursor_y + font_extents.height > window->height())
          {
            // delete last inserted command
            buffer.remove_most_recent_view_update_command();
          }
          redraw = true;
        }
        else if(event.key.keysym.sym == SDLK_UP)
        {
          if(event.key.keysym.mod & KMOD_LSHIFT)
          {
            buffer.execute_selection_command(BufferSelectionCommand::MOVE_UP);
          }
          else
          {
            buffer.execute_cursor_command(BufferCursorCommand::MOVE_UP);
          }
          std::pair<uint32, int32> cursor_coords = buffer.cursor_coords();
          float32 effective_cursor_y =
            scroll_y_offset +
            static_cast<int32>(cursor_coords.first) * font_extents.height;
          if(effective_cursor_y < 0 ||
             effective_cursor_y + font_extents.height > window->height())
          {
            // delete last inserted command
            buffer.remove_most_recent_view_update_command();
          }
          redraw = true;
        }
        else if(event.key.keysym.sym == SDLK_DOWN)
        {
          if(event.key.keysym.mod & KMOD_LSHIFT)
          {
            buffer.execute_selection_command(BufferSelectionCommand::MOVE_DOWN);
          }
          else
          {
            buffer.execute_cursor_command(BufferCursorCommand::MOVE_DOWN);
          }
          std::pair<uint32, int32> cursor_coords = buffer.cursor_coords();
          float32 effective_cursor_y =
            scroll_y_offset +
            static_cast<int32>(cursor_coords.first) * font_extents.height;
          if(effective_cursor_y < 0 ||
             effective_cursor_y + font_extents.height > window->height())
          {
            // delete last inserted command
            buffer.remove_most_recent_view_update_command();
          }
          redraw = true;
        }
        else if(event.key.keysym.sym == SDLK_BACKSPACE)
        {
          buffer.process_backspace();
          tokenizer_cache.update_cache(buffer);
        }
        else if(event.key.keysym.sym == SDLK_TAB)
        {
          buffer.insert_string(std::string(
            ConfigManager::get_instance()->get_config_struct().tab_width, ' '));
          tokenizer_cache.update_cache(buffer);
        }
        else if(event.key.keysym.sym == SDLK_RETURN ||
                event.key.keysym.sym == SDLK_RETURN2)
        {
          buffer.process_enter();
          tokenizer_cache.update_cache(buffer);
        }
        else if(event.key.keysym.sym == SDLK_F11)
        {
          window->toggle_fullscreen();
        }

        // calculating final scroll_y_offset
        std::pair<uint32, int32> cursor_coords = buffer.cursor_coords();
        float32 effective_cursor_y =
          scroll_y_offset +
          static_cast<int32>(cursor_coords.first) * font_extents.height;
        if(effective_cursor_y < 0)
        {
          scroll_y_target = scroll_y_offset =
            -static_cast<int32>(cursor_coords.first) * font_extents.height;
          redraw = true;
        }
        else if(effective_cursor_y + font_extents.height > window->height())
        {
          scroll_y_offset -=
            effective_cursor_y + font_extents.height - window->height();
          scroll_y_target = scroll_y_offset;
          redraw = true;
        }
        redraw = true;
      }
      else if(event.type == SDL_MOUSEBUTTONDOWN)
      {
        // clearing buffer selection
        buffer.clear_selection();

        float32 line_numbers_width =
          (std::to_string(buffer.length()).length() + 2) *
          font_extents.max_x_advance;
        std::pair<uint32, int32> buffer_grid_coords =
          mouse_coords_to_buffer_coords(event.button.x,
                                        event.button.y,
                                        line_numbers_width,
                                        scroll_y_offset,
                                        buffer);

        // setting row
        buffer.set_cursor_row(buffer_grid_coords.first);

        // checking if clicked on line numbers
        if(event.button.x < line_numbers_width)
        {
          buffer.execute_selection_command(BufferSelectionCommand::SELECT_LINE);
        }
        else
        {
          // setting column
          buffer.set_cursor_column(buffer_grid_coords.second);
          buffer.set_cursor_column_target(buffer_grid_coords.second);
        }

        // double click or triple click
        if(event.button.clicks == 2)
        {
          buffer.execute_selection_command(BufferSelectionCommand::SELECT_WORD);
          mouse_double_tap_down = true;
        }
        else if(event.button.clicks == 3)
        {
          buffer.execute_selection_command(BufferSelectionCommand::SELECT_LINE);
          mouse_triple_tap_down = true;
        }
        else
        {
          buffer.set_selection_start_coordinate(buffer_grid_coords);
          mouse_single_tap_down = true;
        }
        //        redraw = true;
      }
      else if(event.type == SDL_MOUSEBUTTONUP)
      {
        mouse_single_tap_down = false;
        mouse_double_tap_down = false;
        mouse_triple_tap_down = false;
      }
      else if(event.type == SDL_TEXTINPUT)
      {
        buffer.insert_string(event.text.text);
        tokenizer_cache.update_cache(buffer);
        redraw = true;
      }
      else if(event.type == SDL_DROPFILE)
      {
        if(buffer.load_from_file(event.drop.file))
        {
          window->title().erase();
          window->title().append(
            "Rocket - " +
            std::filesystem::absolute(std::filesystem::path(event.drop.file))
              .string());
          window->update_title();
          tokenizer_cache.build_cache(buffer);
          scroll_y_offset = 0;
          scroll_y_target = 0;
          redraw = true;
        }
      }
    }

    std::vector<SDL_Rect> rects;
    while(true)
    {
      auto command_result = buffer.get_next_incremental_render_update_command();
      if(command_result == std::nullopt)
      {
        break;
      }

      auto command = command_result.value();
      ExecuteIncrementalRenderUpdate(command,
                                     scroll_y_offset,
                                     font_extents,
                                     window,
                                     buffer,
                                     tokenizer_cache,
                                     rects);
    }
    // window->update_rects(rects.data(), rects.size());

    redraw = redraw ||
             ConfigManager::get_instance()->reload_config_if_changed() ||
             animator(&scroll_y_offset, &scroll_y_target);

    if(redraw)
    {
      window->clear_with_color(hexcode_to_SDL_Color(
        ConfigManager::get_instance()->get_config_struct().colorscheme.bg));

      // rendering line numbers background
      float32 line_numbers_width =
        (std::to_string(buffer.length()).length() + 2) *
        font_extents.max_x_advance;
      RocketRender::rectangle_filled(
        0,
        0,
        line_numbers_width,
        window->height(),
        hexcode_to_SDL_Color(
          ConfigManager::get_instance()->get_config_struct().colorscheme.bg));
      if(ConfigManager::get_instance()->get_config_struct().line_numbers_margin)
      {
        RocketRender::line(line_numbers_width,
                           0,
                           line_numbers_width,
                           window->height(),
                           hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                  ->get_config_struct()
                                                  .colorscheme.gray));
      }

      // drawing contents
      int32 y = scroll_y_offset;
      auto cursor_coord = buffer.cursor_coords();
      uint32 row = 0;
      for(uint32 i = 0; i < buffer.length(); i++)
      {
        if(y < 0 && -y > font_extents.height)
        {
          y += font_extents.height;
          row++;
          continue;
        }

        // drawing line numbers
        std::string number_string = std::move(std::to_string(i + 1));
        RocketRender::text((std::to_string(buffer.length()).length() -
                            number_string.length() + 1) *
                             (font_extents.max_x_advance),
                           ceil(scroll_y_offset + font_extents.height * (i)),
                           number_string,
                           hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                  ->get_config_struct()
                                                  .colorscheme.white));

        if(cursor_coord.first == row)
        {
          // highlight cursor line
          SDL_Color active_line_color =
            hexcode_to_SDL_Color(ConfigManager::get_instance()
                                   ->get_config_struct()
                                   .colorscheme.gray);
          active_line_color.a = 32;
          RocketRender::rectangle_filled(line_numbers_width + 1,
                                         y,
                                         window->width(),
                                         font_extents.height,
                                         active_line_color);
        }
        render_tokens(line_numbers_width + 1,
                      y,
                      *tokenizer_cache.tokens_for_line(i),
                      buffer,
                      i,
                      font_extents);
        y += font_extents.height;
        row++;
        if(y > static_cast<int32>(window->height()))
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
        RocketRender::rectangle_filled_rounded(
          window->width() - scrollbar_width - scrollbar_edge_padding,
          -scroll_y_offset * ratio + scrollbar_edge_padding,
          scrollbar_width,
          scrollbar_height,
          scrollbar_width / 2,
          hexcode_to_SDL_Color(ConfigManager::get_instance()
                                 ->get_config_struct()
                                 .colorscheme.scrollbar));
      }

      // drawing selection
      if(buffer.has_selection())
      {
        auto selection = buffer.selection().value();
        SDL_Color selection_color =
          hexcode_to_SDL_Color(ConfigManager::get_instance()
                                 ->get_config_struct()
                                 .colorscheme.highlight);
        if(selection.first.first == selection.second.first)
        {
          // drawing only selections on single line
          RocketRender::rectangle_filled(
            line_numbers_width + 1 +
              (selection.first.second + 1) * font_extents.max_x_advance,
            ceil(scroll_y_offset + selection.first.first * font_extents.height),
            (selection.second.second - selection.first.second) *
              font_extents.max_x_advance,
            font_extents.height,
            selection_color);
        }
        else
        {
          // multiline selection
          // drawing first line selection
          uint16 selection_width =
            buffer.line_length(selection.first.first).value() -
            selection.first.second;
          RocketRender::rectangle_filled(
            line_numbers_width + 1 +
              (selection.first.second + 1) * font_extents.max_x_advance,
            ceil(scroll_y_offset + selection.first.first * font_extents.height),
            selection_width * font_extents.max_x_advance,
            font_extents.height,
            selection_color);
          // drawing middle lines selection, these lines are fully selected
          for(uint16 line_index = selection.first.first + 1;
              line_index < selection.second.first;
              line_index++)
          {
            RocketRender::rectangle_filled(
              line_numbers_width + 1,
              ceil(scroll_y_offset + line_index * font_extents.height),
              (buffer.line_length(line_index).value() + 1) *
                font_extents.max_x_advance,
              font_extents.height,
              selection_color);
          }
          // drawing last line selection
          RocketRender::rectangle_filled(
            line_numbers_width + 1,
            ceil(scroll_y_offset +
                 selection.second.first * font_extents.height),
            (selection.second.second + 1) * font_extents.max_x_advance,
            font_extents.height,
            selection_color);
        }
      }

      // drawing cursor
      std::pair<uint32, int32> cursor_coords = buffer.cursor_coords();
      std::string cursor_style =
        ConfigManager::get_instance()->get_config_struct().caret.style;
      RocketRender::rectangle_filled(
        line_numbers_width + 1 +
          font_extents.max_x_advance * (cursor_coords.second + 1),
        ceil(scroll_y_offset + font_extents.height * cursor_coords.first),
        (cursor_style == "ibeam" ? ConfigManager::get_instance()
                                     ->get_config_struct()
                                     .caret.ibeam_width
                                 : font_extents.max_x_advance),
        font_extents.height,
        hexcode_to_SDL_Color(
          ConfigManager::get_instance()->get_config_struct().caret.color));

      window->update();
      redraw = false;

      double frame_end_time =
        SDL_GetPerformanceCounter() /
        static_cast<double>(SDL_GetPerformanceFrequency());
      double time_elapsed = frame_end_time - frame_start_time;
      double ideal_frame_time =
        1.0f / ConfigManager::get_instance()->get_config_struct().fps;
      if(time_elapsed >= ideal_frame_time)
      {
        // this souldn't be happening
        // maybe the rendering is taking too much time
        // or the updates
        // if this is occuring frequently
        // then its time to use multithreading
        WARN_BOII("Frame time exceeded ideal frame time by: %lfms!",
                  (time_elapsed - ideal_frame_time) * 1000);
        continue;
      }
      double time_to_sleep = (ideal_frame_time - time_elapsed) * 1000;
      INFO_BOII("Frame time: %lfms, Sleeping for: %lfms",
                time_elapsed * 1000,
                time_to_sleep);
      SDL_Delay(static_cast<uint32>(time_to_sleep));
    }
    else
    {
      window->update_rects(rects.data(), rects.size());
      TRACE_BOII("Updated %d rects", rects.size());
      INFO_BOII("Waiting for event...");
      SDL_WaitEventTimeout(nullptr, wait_time);
    }
  }

cleanup:
  SDL_StopTextInput();
  CursorManager::delete_insance();
  CairoContext::delete_instance();
  delete window;
  SDL_Quit();
  ConfigManager::delete_instance();

  INFO_BOII("Stopped text input");
  return 0;
}
