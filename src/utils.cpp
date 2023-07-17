#include "../include/utils.hpp"

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
