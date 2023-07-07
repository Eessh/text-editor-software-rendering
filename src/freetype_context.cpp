#include "../include/freetype_context.hpp"
#include "../include/macros.hpp"

FreetypeContext* FreetypeContext::_instance = nullptr;

FreetypeContext::FreetypeContext() {}

FreetypeContext::~FreetypeContext() noexcept
{
  FT_Done_Face(_font_face_handle);
  FT_Done_FreeType(_freetype_lib_handle);
}

void FreetypeContext::create_instance() noexcept
{
  if(_instance)
  {
    ERROR_BOII("FreetypeContext is already instantiated, use "
               "FreetypeContext::get_instance()");
    return;
  }

  _instance = new FreetypeContext();
}

FreetypeContext* FreetypeContext::get_instance() noexcept
{
  if(!_instance)
  {
    ERROR_BOII("FreetypeContext instance is not created, use "
               "FreetypeContext::create_instance()");
  }

  return _instance;
}

void FreetypeContext::delete_instance() noexcept
{
  delete _instance;
}

bool FreetypeContext::initialize() noexcept
{
  FT_Error error = FT_Init_FreeType(&_freetype_lib_handle);
  if(error)
  {
    ERROR_BOII("Unable to initialize freetype library, error code: %d", error);
    return false;
  }

  return true;
}

bool FreetypeContext::load_font(const std::string& font_file_path,
                                const uint8& font_size) noexcept
{
  FT_Error error = FT_New_Face(
    _freetype_lib_handle, font_file_path.c_str(), 0, &_font_face_handle);
  if(error == FT_Err_Unknown_File_Format)
  {
    ERROR_BOII("Unable to load font, error: Unknown file format of font file!");
    return false;
  }
  else if(error)
  {
    ERROR_BOII("Unable to read font file!");
    return false;
  }

  FT_Set_Pixel_Sizes(_font_face_handle, 0, font_size);

  return true;
}

FT_Bitmap FreetypeContext::render_char(const char& character) noexcept
{
  FT_UInt glyph_index = FT_Get_Char_Index(_font_face_handle, character);
  FT_Load_Glyph(_font_face_handle, glyph_index, FT_LOAD_DEFAULT);
  FT_Render_Glyph(_font_face_handle->glyph, FT_RENDER_MODE_NORMAL);
  return _font_face_handle->glyph->bitmap;
}