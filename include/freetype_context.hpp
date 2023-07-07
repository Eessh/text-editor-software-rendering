#pragma once

#include <string>
#include "../freetype/ft2build.h"
#include "../freetype/freetype/freetype.h"
#include "../include/types.hpp"

class FreetypeContext
{
public:
  FreetypeContext(const FreetypeContext& context) = delete;
  FreetypeContext(FreetypeContext&& context) = delete;
  FreetypeContext operator=(const FreetypeContext& context) = delete;
  FreetypeContext operator=(FreetypeContext&& context) = delete;

  ~FreetypeContext() noexcept;

  static void create_instance() noexcept;
  static FreetypeContext* get_instance() noexcept;
  static void delete_instance() noexcept;

  bool initialize() noexcept;

  bool load_font(const std::string& font_file_path,
                 const uint8& font_size) noexcept;

  FT_Bitmap render_char(const char& character) noexcept;

private:
  FT_Library _freetype_lib_handle;
  FT_Face _font_face_handle;

  FreetypeContext();

  static FreetypeContext* _instance;
};