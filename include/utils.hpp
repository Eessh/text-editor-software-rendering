#pragma once

#include <string>
#include "sdl2.hpp"

[[nodiscard]] SDL_Color
hexcode_to_SDL_Color(const std::string& hexcode) noexcept;