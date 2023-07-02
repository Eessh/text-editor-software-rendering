#include "../include/cairo_context.hpp"
#include "../include/macros.hpp"

CairoContext* CairoContext::_instance = nullptr;

CairoContext::CairoContext() {}

CairoContext::~CairoContext()
{
  cairo_destroy(_context);
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
  _context = cairo_create(cairo_surface);
  cairo_surface_destroy(cairo_surface);
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
  _context = cairo_create(cairo_surface);
  cairo_surface_destroy(cairo_surface);
}