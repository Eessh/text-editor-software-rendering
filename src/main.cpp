#include <cmath>
#include <fstream>
#include <string>
#include <vector>
#include "../cpp-tokenizer/cpp_tokenizer.hpp"
#include "../include/buffer.hpp"
#include "../include/cairo_context.hpp"
#include "../include/config_manager.hpp"
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

  // Initializing SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
  {
    FATAL_BOII("Unable to initialize SDL: %s", SDL_GetError());
    exit(1);
  }

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
  CppTokenizer::Tokenizer tokenizer;

  // Initial render
  {
    window->clear_with_color(hexcode_to_SDL_Color(
      ConfigManager::get_instance()->get_config_struct().colorscheme.bg));
    float32 y = 0.0f;
    auto cursor_coords = buffer.cursor_coords();
    uint32 row = 0;
    for(const std::string& line : buffer.lines())
    {
      if(y < 0 && -y > font_extents.height)
      {
        y += font_extents.height;
        row++;
        continue;
      }
      if(cursor_coords.first == row)
      {
        // highlight cursor line
        RocketRender::rectangle_filled(
          0, y, 200, font_extents.height, {255, 0, 0, 200});
      }
      RocketRender::text(0, y, line, {0, 0, 0, 255});
      y += font_extents.height;
      row++;
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
        SDL_StopTextInput();
        ConfigManager::delete_instance();
        CursorManager::delete_insance();
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
          if(event.key.keysym.mod & KMOD_LSHIFT)
          {
            buffer.execute_selection_command(BufferSelectionCommand::MOVE_LEFT);
            redraw = true;
          }
          else
          {
            buffer.execute_cursor_command(BufferCursorCommand::MOVE_LEFT);
            std::pair<uint32, int32> cursor_coords = buffer.cursor_coords();
            float32 effective_cursor_y =
              scroll_y_offset +
              static_cast<int32>(cursor_coords.first) * font_extents.height;
            if(effective_cursor_y < 0 ||
               effective_cursor_y + font_extents.height > window->height())
            {
              // delete last inserted command
              buffer.remove_most_recent_command();
            }
          }
        }
        else if(event.key.keysym.sym == SDLK_RIGHT)
        {
          if(event.key.keysym.mod & KMOD_LSHIFT)
          {
            buffer.execute_selection_command(
              BufferSelectionCommand::MOVE_RIGHT);
            redraw = true;
          }
          else
          {
            buffer.execute_cursor_command(BufferCursorCommand::MOVE_RIGHT);
            std::pair<uint32, int32> cursor_coords = buffer.cursor_coords();
            float32 effective_cursor_y =
              scroll_y_offset +
              static_cast<int32>(cursor_coords.first) * font_extents.height;
            if(effective_cursor_y < 0 ||
               effective_cursor_y + font_extents.height > window->height())
            {
              // delete last inserted command
              buffer.remove_most_recent_command();
            }
          }
        }
        else if(event.key.keysym.sym == SDLK_UP)
        {
          if(event.key.keysym.mod & KMOD_LSHIFT)
          {
            buffer.execute_selection_command(BufferSelectionCommand::MOVE_UP);
            redraw = true;
          }
          else
          {
            buffer.execute_cursor_command(BufferCursorCommand::MOVE_UP);
            std::pair<uint32, int32> cursor_coords = buffer.cursor_coords();
            float32 effective_cursor_y =
              scroll_y_offset +
              static_cast<int32>(cursor_coords.first) * font_extents.height;
            if(effective_cursor_y < 0 ||
               effective_cursor_y + font_extents.height > window->height())
            {
              // delete last inserted command
              buffer.remove_most_recent_command();
            }
          }
        }
        else if(event.key.keysym.sym == SDLK_DOWN)
        {
          if(event.key.keysym.mod & KMOD_LSHIFT)
          {
            buffer.execute_selection_command(BufferSelectionCommand::MOVE_DOWN);
            redraw = true;
          }
          else
          {
            buffer.execute_cursor_command(BufferCursorCommand::MOVE_DOWN);
            std::pair<uint32, int32> cursor_coords = buffer.cursor_coords();
            float32 effective_cursor_y =
              scroll_y_offset +
              static_cast<int32>(cursor_coords.first) * font_extents.height;
            if(effective_cursor_y < 0 ||
               effective_cursor_y + font_extents.height > window->height())
            {
              // delete last inserted command
              buffer.remove_most_recent_command();
            }
          }
        }
        else if(event.key.keysym.sym == SDLK_BACKSPACE)
        {
          buffer.process_backspace();
        }
        else if(event.key.keysym.sym == SDLK_RETURN ||
                event.key.keysym.sym == SDLK_RETURN2)
        {
          buffer.process_enter();
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
        // redraw = true;
      }
      else if(event.type == SDL_TEXTINPUT)
      {
        buffer.insert_string(event.text.text);
      }
    }

    std::vector<SDL_Rect> rects;
    while(1)
    {
      auto optional = buffer.get_next_view_update_command();
      if(optional == std::nullopt)
      {
        break;
      }

      auto command = optional.value();
      auto cursor_coord = buffer.cursor_coords();
      if(command.type == BufferViewUpdateCommandType::RENDER_LINE)
      {
        DEBUG_BOII("RENDER_LINE");
        int32 line_y =
          ceil(scroll_y_offset + command.row * font_extents.height);
        tokenizer.clear_tokens();
        std::vector<CppTokenizer::Token> tokens =
          tokenizer.tokenize(buffer.line(command.row).value().get() + "\n");
        // clearing background of line
        // if not cleared, blinking glitch will be happening
        RocketRender::rectangle_filled(
          0,
          line_y,
          window->width(),
          font_extents.height,
          hexcode_to_SDL_Color(
            ConfigManager::get_instance()->get_config_struct().colorscheme.bg));
        // highlighting cursor line
        SDL_Color active_line_color = hexcode_to_SDL_Color(
          ConfigManager::get_instance()->get_config_struct().colorscheme.gray);
        active_line_color.a = 64;
        RocketRender::rectangle_filled(
          0, line_y, window->width(), font_extents.height, active_line_color);
        render_tokens(0, line_y, tokens, font_extents);
        std::string cursor_style =
          ConfigManager::get_instance()->get_config_struct().cursor.style;
        RocketRender::rectangle_filled(
          font_extents.max_x_advance * (cursor_coord.second + 1),
          ceil(scroll_y_offset + font_extents.height * cursor_coord.first),
          (cursor_style == "ibeam" ? ConfigManager::get_instance()
                                       ->get_config_struct()
                                       .cursor.ibeam_width
                                   : font_extents.max_x_advance),
          font_extents.height,
          hexcode_to_SDL_Color(
            ConfigManager::get_instance()->get_config_struct().cursor.color));
        rects.push_back({0,
                         line_y,
                         static_cast<int>(window->width()),
                         static_cast<int>(font_extents.height)});
      }
      else if(command.type == BufferViewUpdateCommandType::RENDER_LINES)
      {
        DEBUG_BOII("RENDER_LINES");
        int32 old_line_y =
          ceil(scroll_y_offset + command.old_active_line * font_extents.height);
        tokenizer.clear_tokens();
        std::vector<CppTokenizer::Token> tokens = tokenizer.tokenize(
          buffer.line(command.old_active_line).value().get() + "\n");
        // clearing background of line
        RocketRender::rectangle_filled(
          0,
          old_line_y,
          window->width(),
          font_extents.height,
          hexcode_to_SDL_Color(
            ConfigManager::get_instance()->get_config_struct().colorscheme.bg));
        render_tokens(0, old_line_y, tokens, font_extents);
        int32 new_line_y =
          ceil(scroll_y_offset + command.new_active_line * font_extents.height);
        // clearing background of line
        RocketRender::rectangle_filled(
          0,
          new_line_y,
          window->width(),
          font_extents.height,
          hexcode_to_SDL_Color(
            ConfigManager::get_instance()->get_config_struct().colorscheme.bg));
        // highlighting cursor line
        SDL_Color active_line_color = hexcode_to_SDL_Color(
          ConfigManager::get_instance()->get_config_struct().colorscheme.gray);
        active_line_color.a = 64;
        RocketRender::rectangle_filled(0,
                                       new_line_y,
                                       window->width(),
                                       font_extents.height,
                                       active_line_color);
        tokenizer.clear_tokens();
        tokens = tokenizer.tokenize(
          buffer.line(command.new_active_line).value().get() + "\n");
        render_tokens(0, new_line_y, tokens, font_extents);
        std::string cursor_style =
          ConfigManager::get_instance()->get_config_struct().cursor.style;
        RocketRender::rectangle_filled(
          font_extents.max_x_advance * (cursor_coord.second + 1),
          ceil(scroll_y_offset + font_extents.height * cursor_coord.first),
          (cursor_style == "ibeam" ? ConfigManager::get_instance()
                                       ->get_config_struct()
                                       .cursor.ibeam_width
                                   : font_extents.max_x_advance),
          font_extents.height,
          hexcode_to_SDL_Color(
            ConfigManager::get_instance()->get_config_struct().cursor.color));
        rects.push_back({0,
                         old_line_y,
                         static_cast<int>(window->width()),
                         static_cast<int>(font_extents.height)});
        rects.push_back({0,
                         new_line_y,
                         static_cast<int>(window->width()),
                         static_cast<int>(font_extents.height)});
      }
      else
      {
        DEBUG_BOII("RENDER_LINE_RANGE");
        // Re-render line range
        int32 line_y =
          ceil(scroll_y_offset + command.start_row * font_extents.height);
        for(uint32 row = command.start_row; row <= command.end_row; row++)
        {
          if(line_y > static_cast<int32>(window->height()))
          {
            break;
          }
          tokenizer.clear_tokens();
          std::vector<CppTokenizer::Token> tokens =
            tokenizer.tokenize(buffer.line(row).value().get() + "\n");
          // clearing background of line
          // if not cleared, blinking glitch will be happening
          RocketRender::rectangle_filled(
            0,
            line_y,
            window->width(),
            font_extents.height,
            hexcode_to_SDL_Color(ConfigManager::get_instance()
                                   ->get_config_struct()
                                   .colorscheme.bg));
          if(cursor_coord.first == row)
          {
            // highlighting cursor line
            SDL_Color active_line_color =
              hexcode_to_SDL_Color(ConfigManager::get_instance()
                                     ->get_config_struct()
                                     .colorscheme.gray);
            active_line_color.a = 64;
            RocketRender::rectangle_filled(0,
                                           line_y,
                                           window->width(),
                                           font_extents.height,
                                           active_line_color);
          }
          render_tokens(0, line_y, tokens, font_extents);
          rects.push_back({0,
                           line_y,
                           static_cast<int>(window->width()),
                           static_cast<int>(font_extents.height)});
          line_y += font_extents.height;
        }
        std::string cursor_style =
          ConfigManager::get_instance()->get_config_struct().cursor.style;
        RocketRender::rectangle_filled(
          font_extents.max_x_advance * (cursor_coord.second + 1),
          ceil(scroll_y_offset + font_extents.height * cursor_coord.first),
          (cursor_style == "ibeam" ? ConfigManager::get_instance()
                                       ->get_config_struct()
                                       .cursor.ibeam_width
                                   : font_extents.max_x_advance),
          font_extents.height,
          hexcode_to_SDL_Color(
            ConfigManager::get_instance()->get_config_struct().cursor.color));
      }
    }
    window->update_rects(rects.data(), rects.size());
    // window->update();

    redraw = redraw || animator(&scroll_y_offset, &scroll_y_target);

    if(redraw)
    {
      // window->clear_with_color({255, 255, 255, 255});
      window->clear_with_color(hexcode_to_SDL_Color(
        ConfigManager::get_instance()->get_config_struct().colorscheme.bg));

      // drawing contents
      int32 y = scroll_y_offset;
      auto cursor_coord = buffer.cursor_coords();
      uint32 row = 0;
      for(const std::string& line : buffer.lines())
      {
        if(y < 0 && -y > font_extents.height)
        {
          y += font_extents.height;
          row++;
          continue;
        }
        if(cursor_coord.first == row)
        {
          // highlighting cursor line
          SDL_Color active_line_color =
            hexcode_to_SDL_Color(ConfigManager::get_instance()
                                   ->get_config_struct()
                                   .colorscheme.gray);
          active_line_color.a = 64;
          RocketRender::rectangle_filled(
            0, y, window->width(), font_extents.height, active_line_color);
        }

        uint32 x = 0;
        tokenizer.clear_tokens();
        std::vector<CppTokenizer::Token> tokens =
          tokenizer.tokenize(line + "\n");
        render_tokens(0, y, tokens, font_extents);
        y += font_extents.height;
        row++;
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
          hexcode_to_SDL_Color(ConfigManager::get_instance()
                                 ->get_config_struct()
                                 .colorscheme.scrollbar));
      }

      // drawing selection
      if(buffer.has_selection())
      {
        auto selection = buffer.selection();
        TRACE_BOII("sorted selection: {(%d, %d), (%d, %d)}",
                   selection.first.first,
                   selection.first.second,
                   selection.second.first,
                   selection.second.second);
        SDL_Color selection_color =
          hexcode_to_SDL_Color(ConfigManager::get_instance()
                                 ->get_config_struct()
                                 .colorscheme.highlight);
        if(selection.first.first == selection.second.first)
        {
          // drawing only selections on single line
          RocketRender::rectangle_filled(
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
              0,
              ceil(scroll_y_offset + row * font_extents.height),
              (buffer.line_length(row).value() + 1) *
                font_extents.max_x_advance,
              font_extents.height,
              selection_color);
          }
          // drawing last line selection
          RocketRender::rectangle_filled(
            0,
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

  SDL_StopTextInput();
  ConfigManager::delete_instance();
  CursorManager::delete_insance();
  CairoContext::delete_instance();
  delete window;
  SDL_Quit();

  return 0;
}

bool animator(float32* animatable, const float32* target) noexcept
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

  *animatable +=
    delta *
    ConfigManager::get_instance()->get_config_struct().scrolling.friction;
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
    else if(token.type == CppTokenizer::TokenType::MULTILINE_COMMENT)
    {
      ERROR_BOII("Rendering multiline comment un-implemented!");
      // RocketRender::text(x, y, token.value, hexcode_to_SDL_Color(ConfigManager::get_instance()->get_config_struct().cpp_token_colors.string));
      // x += token.value.size()*font_extents.max_x_advance;
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