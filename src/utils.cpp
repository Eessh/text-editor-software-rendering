#include "../include/utils.hpp"

[[nodiscard]] SDL_Color
hexcode_to_SDL_Color(const std::string& hexcode) noexcept
{
  SDL_Color parsed_color;

  std::sscanf(hexcode.c_str(),
              "#%02x%02x%02x",
              &parsed_color.r,
              &parsed_color.g,
              &parsed_color.b);
  parsed_color.a = 255;

  return parsed_color;
}
