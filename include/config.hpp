#pragma once

#include <string>
#include "types.hpp"

typedef struct config
{
  uint8 fps;

  bool line_numbers_margin;

  uint8 tab_width;

  bool tab_lines;

  std::string code_font;

  uint8 font_size;

  std::string word_separators;

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

  struct caret
  {
    std::string color, style;
    uint8 ibeam_width;
  } caret;
} config;