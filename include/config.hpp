#pragma once

#include <string>
#include "types.hpp"

typedef struct config
{
  uint8 fps;

  bool line_numbers_margin;

  uint8 tab_width;

  struct window
  {
    uint16 width, height;
  } window;

  struct colorscheme
  {
    std::string bg, fg, red, orange, yellow, green, cyan, blue, purple, white,
      black, gray, highlight, comment, scrollbar;
  } colorscheme;

  struct scrolling
  {
    uint8 sensitivity;
    float32 acceleration;
  } scrolling;

  struct cpp_token_colors
  {
    std::string semicolon, comma, escape_backslash, bracket, square_bracket,
      curly_bracket, character, string, comment, multiline_comment, operator_,
      keyword, preprocessor_directive, identifier, number, function, header;
  } cpp_token_colors;

  struct cursor
  {
    std::string color, style;
    uint8 ibeam_width;
  } cursor;
} config;