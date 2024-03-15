#include "../include/utils.hpp"
#include "../include/cairo_context.hpp"
#include "../include/config_manager.hpp"
#include "../include/rocket_render.hpp"

[[nodiscard]] SDL_Color
hexcode_to_SDL_Color(const std::string& hexcode) noexcept
{
  SDL_Color parsed_color;
  int r, g, b;

  if(hexcode.size() == 7)
  {
    // #RRGGBB hexcode
    std::sscanf(hexcode.c_str(), "#%02x%02x%02x", &r, &g, &b);
    parsed_color.r = r;
    parsed_color.g = g;
    parsed_color.b = b;
    parsed_color.a = 255;
  }
  else if(hexcode.size() == 9)
  {
    // #RRGGBBAA hexcode
    int a;
    std::sscanf(hexcode.c_str(), "#%02x%02x%02x%02x", &r, &g, &b, &a);
    parsed_color.r = r;
    parsed_color.g = g;
    parsed_color.b = b;
    parsed_color.a = a;
  }

  return parsed_color;
}

float32 clamp(const float32 x, const float32 low, const float32 high)
{
  return std::max(std::min(x, high), low);
}

float32
lerp(const float32 low, const float32 high, const float32 interpolated_point)
{
  return low + (high - low) * interpolated_point;
}

bool animator(float32* animatable, const float32* target) noexcept
{
  const float32 delta = *target - *animatable;
  const float32 abs_delta = abs(delta);
  if(delta == 0)
  {
    return false;
  }

  if(abs_delta < 1.0f)
  {
    *animatable += delta;
    return true;
  }

  /// some experiments with scroll animation
  // float32 rate = ConfigManager::get_instance()->get_config_struct().scrolling.acceleration;
  // float32 dt = 60.0f/static_cast<float32>(ConfigManager::get_instance()->get_config_struct().fps);
  // rate = 1 - std::pow(clamp(1-rate, 1e-8, 1-1e-8), 1.0f*dt);
  //
  // *animatable = lerp(*animatable, *target, rate);

  float32 configured_acceleration =
    ConfigManager::get_instance()->get_config_struct().scrolling.acceleration;
  int32 configured_sensitivity =
    ConfigManager::get_instance()->get_config_struct().scrolling.sensitivity;

  /// varying acceleration rates depending upon amount left to scroll
  if(abs_delta < 0.5 * configured_sensitivity)
  {
    configured_acceleration *= 0.5;
  }
  else if(abs_delta < 0.75 * configured_sensitivity)
  {
    configured_acceleration *= 0.75;
  }
  else if(abs_delta > 1.25 * configured_sensitivity)
  {
    configured_acceleration *= 1.25;
  }
  else if(abs_delta > 1.5 * configured_sensitivity)
  {
    configured_acceleration *= 1.5;
  }
  else if(abs_delta > 1.75 * configured_sensitivity)
  {
    configured_acceleration *= 1.75;
  }
  else if(abs_delta > 2 * configured_sensitivity)
  {
    configured_acceleration *= 2;
  }

  *animatable += delta * configured_acceleration;
  return true;
}

void render_tokens(int32 x,
                   int32 y,
                   const std::vector<CppTokenizer::Token>& tokens,
                   const Buffer& buffer,
                   const uint32& line_index,
                   const cairo_font_extents_t& font_extents) noexcept
{
  if(tokens.empty())
  {
    uint8 indent_count =
      buffer.line_tab_indent_count_to_show(line_index).value();
    while(indent_count > 0)
    {
      RocketRender::line(
        x,
        y,
        x,
        y + font_extents.height,
        hexcode_to_SDL_Color(
          ConfigManager::get_instance()->get_config_struct().colorscheme.gray));
      x += ConfigManager::get_instance()->get_config_struct().tab_width *
           font_extents.max_x_advance;
      --indent_count;
    }

    return;
  }

  for(uint32 i = 0; i < tokens.size(); i++)
  {
    CppTokenizer::Token token = tokens[i];
    if(token.value == "\r")
    {
      uint8 indent_count =
        buffer.line_tab_indent_count_to_show(line_index).value();
      while(indent_count > 0)
      {
        RocketRender::line(x,
                           y,
                           x,
                           y + font_extents.height,
                           hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                  ->get_config_struct()
                                                  .colorscheme.gray));
        x += ConfigManager::get_instance()->get_config_struct().tab_width *
             font_extents.max_x_advance;
        --indent_count;
      }
    }
    else if(token.type == CppTokenizer::TokenType::WHITESPACE)
    {
      // drawing tab lines if before token is tab
      if(ConfigManager::get_instance()->get_config_struct().tab_lines &&
         i != 0 && tokens[i - 1].type == CppTokenizer::TokenType::TAB)
      {
        RocketRender::line(x,
                           y,
                           x,
                           y + font_extents.height,
                           hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                  ->get_config_struct()
                                                  .colorscheme.gray));
      }
      x += font_extents.max_x_advance;
    }
    else if(token.type == CppTokenizer::TokenType::TAB)
    {
      // drawing tab lines
      if(ConfigManager::get_instance()->get_config_struct().tab_lines)
      {
        RocketRender::line(x,
                           y,
                           x,
                           y + font_extents.height,
                           hexcode_to_SDL_Color(ConfigManager::get_instance()
                                                  ->get_config_struct()
                                                  .colorscheme.gray));
      }
      x += ConfigManager::get_instance()->get_config_struct().tab_width *
           font_extents.max_x_advance;
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

std::pair<uint32, int32>
mouse_coords_to_buffer_coords(const int& x,
                              const int& y,
                              const float32& x_offset,
                              const float32& scroll_y_offset,
                              const Buffer& buffer) noexcept
{
  uint32 row = (-scroll_y_offset + y) /
               CairoContext::get_instance()->get_font_extents().height;
  if(row > buffer.length() - 1)
  {
    row = buffer.length() - 1;
  }

  if(x < x_offset)
  {
    return std::make_pair(row, static_cast<int32>(-1));
  }

  int32 column = -1;
  float32 max_x_advance =
    CairoContext::get_instance()->get_font_extents().max_x_advance;

  // finding best column based on grid
  int32 left_grid_column = (x - x_offset) / max_x_advance;
  int32 right_grid_column = left_grid_column + 1;
  if(x - x_offset - max_x_advance * left_grid_column <
     max_x_advance * right_grid_column - x + x_offset)
  {
    column = left_grid_column - 1;
  }
  else
  {
    column = right_grid_column - 1;
  }

  if(column > static_cast<int32>(buffer.line_length(row).value()) - 1)
  {
    column = static_cast<int32>(buffer.line_length(row).value()) - 1;
  }

  return std::make_pair(row, column);
}
