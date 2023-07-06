#include "../include/cairo_context.hpp"
#include "../include/macros.hpp"

CairoContext* CairoContext::_instance = nullptr;

CairoContext::CairoContext() {}

CairoContext::~CairoContext()
{
  // Destroying font faces
  for(auto it = _font_map.begin(); it != _font_map.end(); it++)
  {
    cairo_font_face_destroy((*it).second);
  }
  cairo_destroy(_context);
  FT_Done_FreeType(_freetype);
}

void CairoContext::create_instance()
{
  if(_instance)
  {
    ERROR_BOII(
      "CairoContext is already instantiated, use CairoContext::get_instance()");
    return;
  }

  _instance = new CairoContext();
}

CairoContext* CairoContext::get_instance()
{
  return _instance;
}

void CairoContext::delete_instance()
{
  delete _instance;
}

void CairoContext::initialize(Window& window)
{
  SDL_Surface* window_surface = window.surface();
  cairo_surface_t* cairo_surface =
    cairo_image_surface_create_for_data((unsigned char*)window_surface->pixels,
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
}

cairo_t* CairoContext::get_context()
{
  return _context;
}

void CairoContext::reload_context(Window& window)
{
  SDL_Surface* window_surface = window.surface();
  cairo_destroy(_context);
  cairo_surface_t* cairo_surface =
    cairo_image_surface_create_for_data((unsigned char*)window_surface->pixels,
                                        CAIRO_FORMAT_RGB24,
                                        window_surface->w,
                                        window_surface->h,
                                        window_surface->pitch);
  cairo_surface_set_device_scale(cairo_surface, 1, 1);
  _context = cairo_create(cairo_surface);
  cairo_surface_destroy(cairo_surface);
}

bool CairoContext::load_font(const std::string& font_name_to_assign,
                             const std::string& font_file_path)
{
  FT_Face font;
  int error = 0;
  if((error = FT_New_Face(_freetype, font_file_path.c_str(), 0, &font)))
  {
    ERROR_BOII(
      "Unable to load font: %s, error code: %d", font_file_path.c_str(), error);
    return false;
  }

  cairo_font_face_t* ct = cairo_ft_font_face_create_for_ft_face(font, 0);
  _font_map.insert(std::make_pair(font_name_to_assign, ct));
  FT_Done_Face(font);
  return true;
}

bool CairoContext::set_context_font(const std::string& font_name,
                                    const uint8 font_size)
{
  std::unordered_map<std::string, cairo_font_face_t*>::iterator it =
    _font_map.find(font_name);
  if(it == _font_map.end())
  {
    ERROR_BOII("Unable to set context font: %s, maybe it wasn't loaded!",
               font_name.c_str());
    return false;
  }

  cairo_set_font_face(_context, (*it).second);
  cairo_set_font_size(_context, font_size);
  return true;
}