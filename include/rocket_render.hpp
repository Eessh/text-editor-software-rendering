#pragma once

#include <string>
#include "sdl2.hpp"
#include "types.hpp"

namespace RocketRender
{

void rectangle_filled(const int32& x,
                      const int32& y,
                      const uint16& width,
                      const uint16& height,
                      const SDL_Color& color);

void rectangle_outlined(const int32& x,
                        const int32& y,
                        const uint16& width,
                        const uint16& height,
                        const SDL_Color& outline_color);

void rectangle_rounded(const int32& x,
                       const int32& y,
                       const uint16& width,
                       const uint16& height,
                       const uint16& radius,
                       const SDL_Color& color);

void text(const int32& x,
          const int32& y,
          const std::string& text,
          const SDL_Color& color);

}; // namespace RocketRender