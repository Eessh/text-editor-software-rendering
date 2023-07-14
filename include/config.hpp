#pragma once

#include <string>
#include "types.hpp"

typedef struct config
{
  struct window
  {
    uint16 width, height;
  } window;

  struct colorscheme
  {
    std::string bg, fg, red, orange, yellow, green, cyan, blue, purple, white,
      black, gray, highlight, comment;
  } colorscheme;
} config;