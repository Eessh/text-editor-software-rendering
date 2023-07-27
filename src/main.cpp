#include <cmath>
#include <fstream>
#include <string>
#include <vector>
#include "../cpp-tokenizer/cpp_tokenizer.hpp"
#include "../include/buffer.hpp"
#include "../include/cairo_context.hpp"
#include "../include/config_manager.hpp"
#include "../include/cpp_tokenizer_cache.hpp"
#include "../include/cursor_manager.hpp"
#include "../include/macros.hpp"
#include "../include/rocket_render.hpp"
#include "../include/sdl2.hpp"
#include "../include/utils.hpp"
#include "../include/window.hpp"

bool animator(float32* animatable, const float32* target) noexcept;

void render_tokens(int32 x,
                   int32 y,
                   const std::vector<CppTokenizer::Token>& tokens,
                   const cairo_font_extents_t& font_extents) noexcept;

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

  // Creating config manager
  ConfigManager::create_instance();
  if(!ConfigManager::get_instance()->load_config())
  {
    FATAL_BOII("Unable to load config: config.toml!");
    exit(1);
  }

  Window* window = new Window(
    "Text Editor - Software Rendering",
    ConfigManager::get_instance()->get_config_struct().window.width,
    ConfigManager::get_instance()->get_config_struct().window.height);
  window->set_icon("assets/images/rocket.bmp");
  window->set_dark_theme();

  // Creating cairo context
  CairoContext::create_instance();
  CairoContext::get_instance()->initialize(*window);

  // Loading font
  if(!CairoContext::get_instance()->load_font(
       "JetBrainsMono",
       "assets/fonts/JetBrains Mono Regular Nerd Font Complete.ttf"))
  {
    ERROR_BOII("Unable to load font: JetBrainsMono!");
  }
  CairoContext::get_instance()->set_context_font("JetBrainsMono", 16);

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
  bool redraw = true;
  float32 scroll_y_offset = 0.0f, scroll_y_target = 0.0f;
  uint8 scroll_sensitivity = ConfigManager::get_instance()
                               ->get_config_struct()
                               .scrolling.sensitivity,
        wait_time = 250;
  SDL_StartTextInput();
  while(1)
  {
    double frame_start_time =
      SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
    SDL_Event event;
    if(SDL_PollEvent(&event))
    {
      if(event.type == SDL_QUIT)
      {
        goto cleanup;
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
      /// important fix for linux - X11 or Wayland
      /// too many mouse motion events (especially in KDE)
      /// this issue almost made me quit this project
      else if(event.type == SDL_MOUSEMOTION)
      {
        SDL_PumpEvents();
        SDL_Event e;
        while(SDL_PeepEvents(
                &e, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION) > 0)
        {
          // do nothing my boi (for the time being)
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
          if(event.key.keysym.mod & KMOD_LSHIFT)
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
          if(event.key.keysym.mod & KMOD_LSHIFT)
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
        else if(event.key.keysym.sym == SDLK_RETURN ||
                event.key.keysym.sym == SDLK_RETURN2)
        {
          buffer.process_enter();
          tokenizer_cache.update_cache(buffer);
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

        uint32 row = (-scroll_y_offset + event.button.y) /
                     CairoContext::get_instance()->get_font_extents().height;
        int32 column = -1;
        float32 max_x_advance =
          CairoContext::get_instance()->get_font_extents().max_x_advance;
        int32 left_grid_column = event.button.x / max_x_advance;
        int32 right_grid_column = left_grid_column + 1;
        if(event.button.x - max_x_advance * left_grid_column <
           max_x_advance * right_grid_column - event.button.x)
        {
          column = left_grid_column - 1;
        }
        else
        {
          column = right_grid_column - 1;
        }
        if(row > buffer.length() - 1)
        {
          buffer.set_cursor_row(buffer.length() - 1);
        }
        else
        {
          buffer.set_cursor_row(row);
        }
        if(column >
           static_cast<int32>(buffer.line_length(buffer.cursor_row()).value()) -
             1)
        {
          buffer.set_cursor_column(
            static_cast<int32>(
              buffer.line_length(buffer.cursor_row()).value()) -
            1);
        }
        else
        {
          buffer.set_cursor_column(column);
        }

        // double click or triple click
        if(event.button.clicks == 2)
        {
          buffer.execute_selection_command(BufferSelectionCommand::SELECT_WORD);
        }
        else if(event.button.clicks == 3)
        {
          buffer.execute_selection_command(BufferSelectionCommand::SELECT_LINE);
        }
        redraw = true;
      }
      else if(event.type == SDL_TEXTINPUT)
      {
        buffer.insert_string(event.text.text);
        tokenizer_cache.update_cache(buffer);
        redraw = true;
      }
    }

    // std::vector<SDL_Rect> rects;
    // while(1)
    // {
    //   auto optional = buffer.get_next_view_update_command();
    //   if(optional == std::nullopt)
    //   {
    //     break;
    //   }

    //   auto command = optional.value();
    //   auto cursor_coord = buffer.cursor_coords();
    //   if(command.type == BufferViewUpdateCommandType::RENDER_LINE)
    //   {
    //     DEBUG_BOII("RENDER_LINE");
    //     int32 line_y =
    //       ceil(scroll_y_offset + command.row * font_extents.height);
    //     tokenizer.clear_tokens();
    //     std::vector<CppTokenizer::Token> tokens =
    //       tokenizer.tokenize(buffer.line(command.row).value().get() + "\n");
    //     // clearing background of line
    //     // if not cleared, blinking glitch will be happening
    //     RocketRender::rectangle_filled(
    //       0,
    //       line_y,
    //       window->width(),
    //       font_extents.height,
    //       hexcode_to_SDL_Color(
    //         ConfigManager::get_instance()->get_config_struct().colorscheme.bg));
    //     // highlighting cursor line
    //     SDL_Color active_line_color = hexcode_to_SDL_Color(
    //       ConfigManager::get_instance()->get_config_struct().colorscheme.gray);
    //     active_line_color.a = 64;
    //     RocketRender::rectangle_filled(
    //       0, line_y, window->width(), font_extents.height, active_line_color);
    //     render_tokens(0, line_y, tokens, font_extents);
    //     // rendering selection
    //     if(buffer.has_selection())
    //     {
    //       auto opt = buffer.selection_slice_for_line(command.row);
    //       if(opt != std::nullopt)
    //       {
    //         auto selection_line_slice = opt.value();
    //         uint32 selection_length = 0;
    //         if(selection_line_slice.second ==
    //            buffer.line_length(command.row).value() - 1)
    //         {
    //           selection_length = buffer.line_length(command.row).value() -
    //                              selection_line_slice.first;
    //           if(command.row == cursor_coord.first)
    //           {
    //             selection_length -= 1;
    //           }
    //         }
    //         else
    //         {
    //           selection_length =
    //             selection_line_slice.second - selection_line_slice.first;
    //         }
    //         RocketRender::rectangle_filled(
    //           (selection_line_slice.first + 1) * font_extents.max_x_advance,
    //           ceil(scroll_y_offset + command.row * font_extents.height),
    //           selection_length * font_extents.max_x_advance,
    //           font_extents.height,
    //           hexcode_to_SDL_Color(ConfigManager::get_instance()
    //                                  ->get_config_struct()
    //                                  .colorscheme.highlight));
    //       }
    //     }
    //     // rendering cursor
    //     std::string cursor_style =
    //       ConfigManager::get_instance()->get_config_struct().cursor.style;
    //     RocketRender::rectangle_filled(
    //       font_extents.max_x_advance * (cursor_coord.second + 1),
    //       ceil(scroll_y_offset + font_extents.height * cursor_coord.first),
    //       (cursor_style == "ibeam" ? ConfigManager::get_instance()
    //                                    ->get_config_struct()
    //                                    .cursor.ibeam_width
    //                                : font_extents.max_x_advance),
    //       font_extents.height,
    //       hexcode_to_SDL_Color(
    //         ConfigManager::get_instance()->get_config_struct().cursor.color));
    //     rects.push_back((SDL_Rect){0,
    //                                static_cast<int>(line_y),
    //                                static_cast<int>(window->width()),
    //                                static_cast<int>(font_extents.height)});
    //   }
    //   else if(command.type == BufferViewUpdateCommandType::RENDER_LINES)
    //   {
    //     DEBUG_BOII("RENDER_LINES");
    //     int32 old_line_y =
    //       ceil(scroll_y_offset + command.old_active_line * font_extents.height);
    //     // render old line only if it is inside viewport
    //     if(0 <= static_cast<int32>(old_line_y + font_extents.height) &&
    //        old_line_y <= static_cast<int32>(window->height()))
    //     {
    //       tokenizer.clear_tokens();
    //       std::vector<CppTokenizer::Token> tokens = tokenizer.tokenize(
    //         buffer.line(command.old_active_line).value().get() + "\n");
    //       // clearing background of line
    //       RocketRender::rectangle_filled(
    //         0,
    //         old_line_y,
    //         window->width(),
    //         font_extents.height,
    //         hexcode_to_SDL_Color(ConfigManager::get_instance()
    //                                ->get_config_struct()
    //                                .colorscheme.bg));
    //       render_tokens(0, old_line_y, tokens, font_extents);
    //       // rendering selection
    //       if(buffer.has_selection())
    //       {
    //         auto opt = buffer.selection_slice_for_line(command.old_active_line);
    //         if(opt != std::nullopt)
    //         {
    //           auto selection_line_slice = opt.value();
    //           uint32 selection_length = 0;
    //           if(selection_line_slice.second ==
    //              buffer.line_length(command.old_active_line).value() - 1)
    //           {
    //             selection_length =
    //               buffer.line_length(command.old_active_line).value() -
    //               selection_line_slice.first;
    //           }
    //           else
    //           {
    //             selection_length =
    //               selection_line_slice.second - selection_line_slice.first;
    //           }
    //           RocketRender::rectangle_filled(
    //             (selection_line_slice.first + 1) * font_extents.max_x_advance,
    //             ceil(scroll_y_offset +
    //                  command.old_active_line * font_extents.height),
    //             selection_length * font_extents.max_x_advance,
    //             font_extents.height,
    //             hexcode_to_SDL_Color(ConfigManager::get_instance()
    //                                    ->get_config_struct()
    //                                    .colorscheme.highlight));
    //         }
    //       }
    //     }
    //     int32 new_line_y =
    //       ceil(scroll_y_offset + command.new_active_line * font_extents.height);
    //     // clearing background of line
    //     RocketRender::rectangle_filled(
    //       0,
    //       new_line_y,
    //       window->width(),
    //       font_extents.height,
    //       hexcode_to_SDL_Color(
    //         ConfigManager::get_instance()->get_config_struct().colorscheme.bg));
    //     // highlighting cursor line
    //     SDL_Color active_line_color = hexcode_to_SDL_Color(
    //       ConfigManager::get_instance()->get_config_struct().colorscheme.gray);
    //     active_line_color.a = 64;
    //     RocketRender::rectangle_filled(0,
    //                                    new_line_y,
    //                                    window->width(),
    //                                    font_extents.height,
    //                                    active_line_color);
    //     tokenizer.clear_tokens();
    //     std::vector<CppTokenizer::Token> tokens = tokenizer.tokenize(
    //       buffer.line(command.new_active_line).value().get() + "\n");
    //     render_tokens(0, new_line_y, tokens, font_extents);
    //     // rendering selection
    //     if(buffer.has_selection())
    //     {
    //       auto opt = buffer.selection_slice_for_line(command.new_active_line);
    //       if(opt != std::nullopt)
    //       {
    //         auto selection_line_slice = opt.value();
    //         uint32 selection_length = 0;
    //         if(selection_line_slice.second ==
    //            buffer.line_length(command.new_active_line).value() - 1)
    //         {
    //           selection_length =
    //             buffer.line_length(command.new_active_line).value() -
    //             selection_line_slice.first;
    //           if(command.new_active_line == cursor_coord.first)
    //           {
    //             selection_length -= 1;
    //           }
    //         }
    //         else
    //         {
    //           selection_length =
    //             selection_line_slice.second - selection_line_slice.first;
    //         }
    //         RocketRender::rectangle_filled(
    //           (selection_line_slice.first + 1) * font_extents.max_x_advance,
    //           ceil(scroll_y_offset +
    //                command.new_active_line * font_extents.height),
    //           selection_length * font_extents.max_x_advance,
    //           font_extents.height,
    //           hexcode_to_SDL_Color(ConfigManager::get_instance()
    //                                  ->get_config_struct()
    //                                  .colorscheme.highlight));
    //       }
    //     }
    //     std::string cursor_style =
    //       ConfigManager::get_instance()->get_config_struct().cursor.style;
    //     RocketRender::rectangle_filled(
    //       font_extents.max_x_advance * (cursor_coord.second + 1),
    //       ceil(scroll_y_offset + font_extents.height * cursor_coord.first),
    //       (cursor_style == "ibeam" ? ConfigManager::get_instance()
    //                                    ->get_config_struct()
    //                                    .cursor.ibeam_width
    //                                : font_extents.max_x_advance),
    //       font_extents.height,
    //       hexcode_to_SDL_Color(
    //         ConfigManager::get_instance()->get_config_struct().cursor.color));
    //     rects.push_back((SDL_Rect){0,
    //                                static_cast<int>(old_line_y),
    //                                static_cast<int>(window->width()),
    //                                static_cast<int>(font_extents.height)});
    //     rects.push_back((SDL_Rect){0,
    //                                static_cast<int>(new_line_y),
    //                                static_cast<int>(window->width()),
    //                                static_cast<int>(font_extents.height)});
    //   }
    //   else
    //   {
    //     DEBUG_BOII("RENDER_LINE_RANGE");
    //     // Re-render line range
    //     int32 line_y =
    //       ceil(scroll_y_offset + command.start_row * font_extents.height);
    //     for(uint32 row = command.start_row; row <= command.end_row; row++)
    //     {
    //       if(line_y > static_cast<int32>(window->height()))
    //       {
    //         break;
    //       }
    //       if(row >= buffer.length())
    //       {
    //         DEBUG_BOII("Deleted line: %ld, y: %ld", row, line_y);
    //         // clear background, continue
    //         RocketRender::rectangle_filled(
    //           0,
    //           line_y,
    //           window->width(),
    //           font_extents.height,
    //           hexcode_to_SDL_Color(ConfigManager::get_instance()
    //                                  ->get_config_struct()
    //                                  .colorscheme.bg));
    //         rects.push_back((SDL_Rect){0,
    //                                    static_cast<int>(line_y),
    //                                    static_cast<int>(window->width()),
    //                                    static_cast<int>(font_extents.height)});
    //         line_y += font_extents.height;
    //         continue;
    //       }
    //       tokenizer.clear_tokens();
    //       std::vector<CppTokenizer::Token> tokens =
    //         tokenizer.tokenize(buffer.line(row).value().get() + "\n");
    //       // clearing background of line
    //       // if not cleared, blinking glitch will be happening
    //       RocketRender::rectangle_filled(
    //         0,
    //         line_y,
    //         window->width(),
    //         font_extents.height,
    //         hexcode_to_SDL_Color(ConfigManager::get_instance()
    //                                ->get_config_struct()
    //                                .colorscheme.bg));
    //       if(cursor_coord.first == row)
    //       {
    //         // highlighting cursor line
    //         SDL_Color active_line_color =
    //           hexcode_to_SDL_Color(ConfigManager::get_instance()
    //                                  ->get_config_struct()
    //                                  .colorscheme.gray);
    //         active_line_color.a = 64;
    //         RocketRender::rectangle_filled(0,
    //                                        line_y,
    //                                        window->width(),
    //                                        font_extents.height,
    //                                        active_line_color);
    //       }
    //       render_tokens(0, line_y, tokens, font_extents);
    //       // rendering selection
    //       if(buffer.has_selection())
    //       {
    //         auto opt = buffer.selection_slice_for_line(row);
    //         if(opt != std::nullopt)
    //         {
    //           auto selection_line_slice = opt.value();
    //           uint32 selection_length = 0;
    //           if(selection_line_slice.second ==
    //              buffer.line_length(row).value() - 1)
    //           {
    //             selection_length =
    //               buffer.line_length(row).value() - selection_line_slice.first;
    //             if(row == cursor_coord.first)
    //             {
    //               selection_length -= 1;
    //             }
    //           }
    //           else
    //           {
    //             selection_length =
    //               selection_line_slice.second - selection_line_slice.first;
    //           }
    //           RocketRender::rectangle_filled(
    //             (selection_line_slice.first + 1) * font_extents.max_x_advance,
    //             ceil(scroll_y_offset + row * font_extents.height),
    //             selection_length * font_extents.max_x_advance,
    //             font_extents.height,
    //             hexcode_to_SDL_Color(ConfigManager::get_instance()
    //                                    ->get_config_struct()
    //                                    .colorscheme.highlight));
    //         }
    //       }
    //       rects.push_back((SDL_Rect){0,
    //                                  static_cast<int>(line_y),
    //                                  static_cast<int>(window->width()),
    //                                  static_cast<int>(font_extents.height)});
    //       line_y += font_extents.height;
    //     }
    //     std::string cursor_style =
    //       ConfigManager::get_instance()->get_config_struct().cursor.style;
    //     RocketRender::rectangle_filled(
    //       font_extents.max_x_advance * (cursor_coord.second + 1),
    //       ceil(scroll_y_offset + font_extents.height * cursor_coord.first),
    //       (cursor_style == "ibeam" ? ConfigManager::get_instance()
    //                                    ->get_config_struct()
    //                                    .cursor.ibeam_width
    //                                : font_extents.max_x_advance),
    //       font_extents.height,
    //       hexcode_to_SDL_Color(
    //         ConfigManager::get_instance()->get_config_struct().cursor.color));
    //   }
    // }
    // window->update_rects(rects.data(), rects.size());

    redraw = redraw || animator(&scroll_y_offset, &scroll_y_target);

    if(redraw)
    {
      window->clear_with_color(hexcode_to_SDL_Color(
        ConfigManager::get_instance()->get_config_struct().colorscheme.bg));

      // rendering line numbers
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
        RocketRender::line(line_numbers_width - 1,
                           0,
                           line_numbers_width - 1,
                           window->height(),
                           hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                  ->get_config_struct()
                                                  .colorscheme.gray));
      }
      for(uint32 i = 1; i <= buffer.length(); i++)
      {
        std::string string_to_render = std::move(std::to_string(i));
        while(string_to_render.length() !=
              std::to_string(buffer.length()).length())
        {
          string_to_render.insert(0, " ");
        }
        RocketRender::text(font_extents.max_x_advance,
                           scroll_y_offset + font_extents.height * (i - 1),
                           string_to_render,
                           hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                  ->get_config_struct()
                                                  .colorscheme.white));
      }

      // drawing contents
      int32 y = scroll_y_offset;
      auto cursor_coord = buffer.cursor_coords();
      uint32 row = 0;
      for(const std::vector<CppTokenizer::Token>& line_tokens :
          tokenizer_cache.tokens())
      {
        if(y < 0 && -y > font_extents.height)
        {
          y += font_extents.height;
          row++;
          continue;
        }
        if(cursor_coord.first == row)
        {
          // highlight cursor line
          SDL_Color active_line_color =
            hexcode_to_SDL_Color(ConfigManager::get_instance()
                                   ->get_config_struct()
                                   .colorscheme.gray);
          active_line_color.a = 64;
          RocketRender::rectangle_filled(line_numbers_width + 1,
                                         y,
                                         window->width(),
                                         font_extents.height,
                                         active_line_color);
        }
        render_tokens(line_numbers_width + 1, y, line_tokens, font_extents);
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
        RocketRender::rectangle_rounded(
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
        auto selection = buffer.selection();
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
          for(uint16 row = selection.first.first + 1;
              row < selection.second.first;
              row++)
          {
            RocketRender::rectangle_filled(
              line_numbers_width + 1,
              ceil(scroll_y_offset + row * font_extents.height),
              (buffer.line_length(row).value() + 1) *
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
        ConfigManager::get_instance()->get_config_struct().cursor.style;
      RocketRender::rectangle_filled(
        line_numbers_width + 1 +
          font_extents.max_x_advance * (cursor_coords.second + 1),
        ceil(scroll_y_offset + font_extents.height * cursor_coords.first),
        (cursor_style == "ibeam" ? ConfigManager::get_instance()
                                     ->get_config_struct()
                                     .cursor.ibeam_width
                                 : font_extents.max_x_advance),
        font_extents.height,
        hexcode_to_SDL_Color(
          ConfigManager::get_instance()->get_config_struct().cursor.color));

      window->update();
      redraw = false;

      double frame_end_time =
        SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
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
      uint32 time_to_delay = (ideal_frame_time - time_elapsed) * 1000;
      INFO_BOII("Frame time: %lf, Delay: %ldms", time_elapsed, time_to_delay);
      SDL_Delay(time_to_delay);
    }
    else
    {
      INFO_BOII("Waiting for event...");
      SDL_WaitEventTimeout(NULL, wait_time);
    }
  }

cleanup:
  SDL_StopTextInput();
  ConfigManager::delete_instance();
  CursorManager::delete_insance();
  CairoContext::delete_instance();
  delete window;
  SDL_Quit();

  return 0;
}
/*
float32 clamp(const float32 x, const float32 low, const float32 high)
{
  return std::max(std::min(x, high), low);
}

float32
lerp(const float32 low, const float32 high, const float32 interpolated_point)
{
  return low + (high - low) * interpolated_point;
}
*/
bool animator(float32* animatable, const float32* target) noexcept
{
  const float32 delta = *target - *animatable;
  if(delta == 0)
  {
    return false;
  }

  if(abs(delta) < 0.5f)
  {
    *animatable += delta;
    return true;
  }

  /// some experiments with scroll animation
  // float32 rate = 0.5;
  // float32 dt = 60.0f/static_cast<float32>(ConfigManager::get_instance()->get_config_struct().fps);
  // rate = 1 - std::pow(clamp(1-rate, 1e-8, 1-1e-8), 1.0f*dt);

  // *animatable = lerp(*animatable, *target, rate);

  *animatable +=
    delta *
    ConfigManager::get_instance()->get_config_struct().scrolling.acceleration;
  return true;
}

void render_tokens(int32 x,
                   int32 y,
                   const std::vector<CppTokenizer::Token>& tokens,
                   const cairo_font_extents_t& font_extents) noexcept
{
  for(const CppTokenizer::Token& token : tokens)
  {
    if(token.value == "\r")
    {
      // do nothing
    }
    else if(token.type == CppTokenizer::TokenType::WHITESPACE ||
            token.type == CppTokenizer::TokenType::TAB)
    {
      x += font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::SEMICOLON)
    {
      RocketRender::text(x,
                         y,
                         token.value,
                         hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                ->get_config_struct()
                                                .cpp_token_colors.semicolon));
      x += font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::COMMA)
    {
      RocketRender::text(x,
                         y,
                         token.value,
                         hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                ->get_config_struct()
                                                .cpp_token_colors.comma));
      x += font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::ESCAPE_BACKSLASH)
    {
      RocketRender::text(
        x,
        y,
        token.value,
        hexcode_to_SDL_Color(ConfigManager::get_instance()
                               ->get_config_struct()
                               .cpp_token_colors.escape_backslash));
      x += font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::BRACKET_OPEN ||
            token.type == CppTokenizer::TokenType::BRACKET_CLOSE)
    {
      RocketRender::text(x,
                         y,
                         token.value,
                         hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                ->get_config_struct()
                                                .cpp_token_colors.bracket));
      x += font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::SQUARE_BRACKET_OPEN ||
            token.type == CppTokenizer::TokenType::SQUARE_BRACKET_CLOSE)
    {
      RocketRender::text(
        x,
        y,
        token.value,
        hexcode_to_SDL_Color(ConfigManager::get_instance()
                               ->get_config_struct()
                               .cpp_token_colors.square_bracket));
      x += font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::CURLY_BRACE_OPEN ||
            token.type == CppTokenizer::TokenType::CURLY_BRACE_CLOSE)
    {
      RocketRender::text(
        x,
        y,
        token.value,
        hexcode_to_SDL_Color(ConfigManager::get_instance()
                               ->get_config_struct()
                               .cpp_token_colors.curly_bracket));
      x += font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::CHARACTER)
    {
      RocketRender::text(x,
                         y,
                         token.value,
                         hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                ->get_config_struct()
                                                .cpp_token_colors.character));
      x += token.value.size() * font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::STRING)
    {
      RocketRender::text(x,
                         y,
                         token.value,
                         hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                ->get_config_struct()
                                                .cpp_token_colors.string));
      x += token.value.size() * font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::COMMENT)
    {
      RocketRender::text(x,
                         y,
                         token.value,
                         hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                ->get_config_struct()
                                                .cpp_token_colors.comment));
      x += token.value.size() * font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::MULTILINE_COMMENT ||
            token.type == CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE)
    {
      std::string trimmed_token = token.value;
      if(trimmed_token.back() == '\n')
      {
        trimmed_token.pop_back();
      }
      RocketRender::text(x,
                         y,
                         trimmed_token,
                         hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                ->get_config_struct()
                                                .cpp_token_colors.comment));
      x += trimmed_token.size() * font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::OPERATOR)
    {
      RocketRender::text(x,
                         y,
                         token.value,
                         hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                ->get_config_struct()
                                                .cpp_token_colors.operator_));
      x += token.value.size() * font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::KEYWORD)
    {
      RocketRender::text(x,
                         y,
                         token.value,
                         hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                ->get_config_struct()
                                                .cpp_token_colors.keyword));
      x += token.value.size() * font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::PREPROCESSOR_DIRECTIVE)
    {
      RocketRender::text(
        x,
        y,
        token.value,
        hexcode_to_SDL_Color(ConfigManager::get_instance()
                               ->get_config_struct()
                               .cpp_token_colors.preprocessor_directive));
      x += token.value.size() * font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::IDENTIFIER)
    {
      RocketRender::text(x,
                         y,
                         token.value,
                         hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                ->get_config_struct()
                                                .cpp_token_colors.identifier));
      x += token.value.size() * font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::NUMBER)
    {
      RocketRender::text(x,
                         y,
                         token.value,
                         hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                ->get_config_struct()
                                                .cpp_token_colors.number));
      x += token.value.size() * font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::FUNCTION)
    {
      RocketRender::text(x,
                         y,
                         token.value,
                         hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                ->get_config_struct()
                                                .cpp_token_colors.function));
      x += token.value.size() * font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::HEADER)
    {
      RocketRender::text(x,
                         y,
                         token.value,
                         hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                ->get_config_struct()
                                                .cpp_token_colors.header));
      x += token.value.size() * font_extents.max_x_advance;
    }
  }
}