#include "../include/cairo_context.hpp"
#include "../include/macros.hpp"

CairoContext* CairoContext::_instance = nullptr;

CairoContext::CairoContext() {}

CairoContext::~CairoContext() noexcept
{
  // Destroying font faces
  for(auto& it : _font_map)
  {
    cairo_font_face_destroy(it.second.second);
    FT_Done_Face(it.second.first);
  }
  cairo_destroy(_context);
  FT_Done_FreeType(_freetype);
}

void CairoContext::create_instance() noexcept
{
  if(_instance)
  {
    ERROR_BOII(
      "CairoContext is already instantiated, use CairoContext::get_instance()");
    return;
  }

  _instance = new CairoContext();
}

CairoContext* CairoContext::get_instance() noexcept
{
  return _instance;
}

void CairoContext::delete_instance() noexcept
{
  delete _instance;
}

void CairoContext::initialize(Window& window) noexcept
{
  SDL_Surface* window_surface = window.surface();
  cairo_surface_t* cairo_surface = cairo_image_surface_create_for_data(
    static_cast<unsigned char*>(window_surface->pixels),
    CAIRO_FORMAT_RGB24,
    window_surface->w,
    window_surface->h,
    window_surface->pitch);
  cairo_surface_set_device_scale(cairo_surface, 1, 1);
  _context = cairo_create(cairo_surface);
  cairo_surface_destroy(cairo_surface);

  int error = 0;
  if((error = FT_Init_FreeType(&_freetype)))
  {
    ERROR_BOII("Unable to initialize freetype: %d", error);
  }
  unsigned char weights[] = {0x10, 0x40, 0x70, 0x40, 0x10};
  FT_Library_SetLcdFilterWeights(_freetype, weights);
}

cairo_t* CairoContext::get_context() const noexcept
{
  return _context;
}

void CairoContext::reload_context(Window& window) noexcept
{
  cairo_destroy(_context);
  SDL_Surface* window_surface = window.surface();
  cairo_surface_t* cairo_surface = cairo_image_surface_create_for_data(
    static_cast<unsigned char*>(window_surface->pixels),
    CAIRO_FORMAT_RGB24,
    window_surface->w,
    window_surface->h,
    window_surface->pitch);
  cairo_surface_set_device_scale(cairo_surface, 1.0, 1.0);
  _context = cairo_create(cairo_surface);
  cairo_surface_destroy(cairo_surface);

  // setting previous fonts
  cairo_set_font_face(_context, _active_font_face);
  cairo_set_font_size(_context, _active_font_size);
}

bool CairoContext::load_font(const std::string& font_name_to_assign,
                             const std::string& font_file_path) noexcept
{
  FT_Face font;
  int error = 0;
  if((error = FT_New_Face(_freetype, font_file_path.c_str(), 0, &font)))
  {
    ERROR_BOII(
      "Unable to load font: %s, error code: %d", font_file_path.c_str(), error);
    return false;
  }

  cairo_font_face_t* ct =
    cairo_ft_font_face_create_for_ft_face(font, FT_LOAD_FORCE_AUTOHINT);
  _font_map.insert({font_name_to_assign, std::make_pair(font, ct)});
  return true;
}

bool CairoContext::set_context_font(const std::string& font_name,
                                    const uint8& font_size) noexcept
{
  auto it = _font_map.find(font_name);
  if(it == _font_map.end())
  {
    ERROR_BOII("Unable to set context font: %s, maybe it wasn't loaded!",
               font_name.c_str());
    return false;
  }

  cairo_set_font_face(_context, it->second.second);
  cairo_set_font_size(_context, font_size);

  _active_font_face = it->second.second;
  _active_font_size = font_size;

  return true;
}

cairo_font_extents_t CairoContext::get_font_extents() const noexcept
{
  if(_font_map.empty())
  {
    WARN_BOII("No font is loaded, using cairo's fallback font!");
  }

  cairo_font_extents_t font_extents;
  cairo_font_extents(_context, &font_extents);

  return font_extents;
}

cairo_text_extents_t
CairoContext::get_text_extents(const std::string& text) const noexcept
{
  if(_font_map.empty())
  {
    WARN_BOII("No font is loaded, using cairo's fallback font!");
  }

  cairo_text_extents_t text_extents;
  cairo_text_extents(_context, text.c_str(), &text_extents);

  return text_extents;
}