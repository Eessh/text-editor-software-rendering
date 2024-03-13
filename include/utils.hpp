#pragma once

#include <string>
#include <vector>
#include "buffer.hpp"
#include "cairo.hpp"
#include "sdl2.hpp"
#include "types.hpp"

[[nodiscard]] SDL_Color
hexcode_to_SDL_Color(const std::string& hexcode) noexcept;

bool animator(float32* animatable, const float32* target) noexcept;

void render_tokens(int32 x,
                   int32 y,
                   const std::vector<CppTokenizer::Token>& tokens,
                   const Buffer& buffer,
                   const uint32& line_index,
                   const cairo_font_extents_t& font_extents) noexcept;

/// @brief Gives buffer grid position from mouse coordinates.
/// @param x x-coordinate of mouse.
/// @param y y-coordinate of mouse.
/// @param x_offset offset for x-coordinate (ex: line numbers column).
/// @param scroll_y_offset offset for y-coodinate (ex: y - scroll offset).
/// @param buffer const reference to buffer.
/// @return Returns pair of grid coorinates in buffer.
std::pair<uint32, int32>
mouse_coords_to_buffer_coords(const int& x,
                              const int& y,
                              const float32& x_offset,
                              const float32& scroll_y_offset,
                              const Buffer& buffer) noexcept;
