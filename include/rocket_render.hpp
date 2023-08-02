#pragma once

#include <string>
#include "sdl2.hpp"
#include "types.hpp"

/// @brief Rocket Render - consists of primitive drawing utilities.
namespace RocketRender
{

/// @brief Draws line between two points.
/// @param x1 x-coordinate of starting point.
/// @param y1 y-coordinate of starting point.
/// @param x2 x-coordinate of ending point.
/// @param y2 y-coordinate of ending point.
/// @param color color of line.
void line(const int32& x1,
          const int32& y1,
          const int32& x2,
          const int32& y2,
          const SDL_Color& color);

/// @brief Draws filled rectange.
/// @param x x-coordinate of top-left corner of rectangle.
/// @param y y-coordinate of top-left corner of rectangle.
/// @param width width of rectangle.
/// @param height height of rectangle.
/// @param color color of rectangle.
void rectangle_filled(const int32& x,
                      const int32& y,
                      const uint16& width,
                      const uint16& height,
                      const SDL_Color& color);

/// @brief Draws outlined rectangle.
/// @param x x-coordinate of top-left corner of rectangle.
/// @param y y-coordinate of top-left corner of rectangle.
/// @param width width of rectangle.
/// @param height height of rectangle.
/// @param outline_color outline color of rectangle.
void rectangle_outlined(const int32& x,
                        const int32& y,
                        const uint16& width,
                        const uint16& height,
                        const SDL_Color& outline_color);

/// @brief Draws rounded filled rectangle.
/// @param x x-coordinate of top-left corner of rectangle.
/// @param y y-coordinate of top-left corner of rectangle.
/// @param width width of rectangle.
/// @param height height of rectangle.
/// @param radius corner radius of rectangle.
/// @param color color of rectangle.
void rectangle_filled_rounded(const int32& x,
                              const int32& y,
                              const uint16& width,
                              const uint16& height,
                              const uint16& radius,
                              const SDL_Color& color);

/// @brief Draws rounded outlined rectangle.
/// @param x x-coordinate of top-left corner of rectangle.
/// @param y y-coordinate of top-left corner of rectangle.
/// @param width width of rectangle.
/// @param height height of rectangle.
/// @param radius corner radius of rectangle.
/// @param outline_color outline color of rectangle.
void rectangle_outlined_rounded(const int32& x,
                                const int32& y,
                                const uint16& width,
                                const uint16& height,
                                const uint16& radius,
                                const SDL_Color& outline_color);

/// @brief Draws text.
/// @param x x-coordinate of top-left corner.
/// @param y y-coordinate of top-left corner.
/// @param text text to render.
/// @param color color of text.
void text(const int32& x,
          const int32& y,
          const std::string& text,
          const SDL_Color& color);

}; // namespace RocketRender