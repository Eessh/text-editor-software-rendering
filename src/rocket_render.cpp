#include "../include/rocket_render.hpp"
#include "../include/cairo_context.hpp"

void RocketRender::line(const int32& x1,
                        const int32& y1,
                        const int32& x2,
                        const int32& y2,
                        const SDL_Color& color)
{
  cairo_t* cr = CairoContext::get_instance()->get_context();
  cairo_set_source_rgba(cr,
                        (float)color.r / 255.0,
                        (float)color.g / 255.0,
                        (float)color.b / 255.0,
                        (float)color.a / 255.0);
  cairo_move_to(cr, x1 + 0.5f, y1 + 0.5f);
  cairo_set_line_width(cr, 0.5);
  cairo_line_to(cr, x2 + 0.5f, y2 + 0.5f);
  cairo_stroke(cr);
  cairo_close_path(cr);
}

void RocketRender::rectangle_filled(const int32& x,
                                    const int32& y,
                                    const uint16& width,
                                    const uint16& height,
                                    const SDL_Color& color)
{
  cairo_t* cr = CairoContext::get_instance()->get_context();
  cairo_set_source_rgba(cr,
                        (float)color.r / 255.0,
                        (float)color.g / 255.0,
                        (float)color.b / 255.0,
                        (float)color.a / 255.0);
  cairo_rectangle(cr, x, y, width, height);
  cairo_fill(cr);
}

void RocketRender::rectangle_outlined(const int32& x,
                                      const int32& y,
                                      const uint16& width,
                                      const uint16& height,
                                      const SDL_Color& outline_color)
{
  cairo_t* cr = CairoContext::get_instance()->get_context();
  cairo_set_source_rgba(cr,
                        (float)outline_color.r / 255.0,
                        (float)outline_color.g / 255.0,
                        (float)outline_color.b / 255.0,
                        (float)outline_color.a / 255.0);
  cairo_rectangle(cr, x, y, width, height);
  cairo_stroke(cr);
}

void RocketRender::rectangle_filled_rounded(const int32& x,
                                            const int32& y,
                                            const uint16& width,
                                            const uint16& height,
                                            const uint16& radius,
                                            const SDL_Color& color)
{
  cairo_t* cr = CairoContext::get_instance()->get_context();
  cairo_set_source_rgba(cr,
                        (float)color.r / 255.0,
                        (float)color.g / 255.0,
                        (float)color.b / 255.0,
                        (float)color.a / 255.0);
  cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
  cairo_new_sub_path(cr);
  cairo_arc(cr, x + radius, y + radius, radius, M_PI, 3 * M_PI / 2);
  cairo_arc(cr, x + width - radius, y + radius, radius, 3 * M_PI / 2, 2 * M_PI);
  cairo_arc(cr, x + width - radius, y + height - radius, radius, 0, M_PI / 2);
  cairo_arc(cr, x + radius, y + height - radius, radius, M_PI / 2, M_PI);
  cairo_close_path(cr);
  cairo_fill(cr);
}

void RocketRender::rectangle_outlined_rounded(const int32& x,
                                              const int32& y,
                                              const uint16& width,
                                              const uint16& height,
                                              const uint16& radius,
                                              const SDL_Color& outline_color)
{
  cairo_t* cr = CairoContext::get_instance()->get_context();
  cairo_set_source_rgba(cr,
                        (float)outline_color.r / 255.0,
                        (float)outline_color.g / 255.0,
                        (float)outline_color.b / 255.0,
                        (float)outline_color.a / 255.0);
  cairo_new_sub_path(cr);
  cairo_arc(cr, x + radius, y + radius, radius, M_PI, 3 * M_PI / 2);
  cairo_arc(cr, x + width - radius, y + radius, radius, 3 * M_PI / 2, 2 * M_PI);
  cairo_arc(cr, x + width - radius, y + height - radius, radius, 0, M_PI / 2);
  cairo_arc(cr, x + radius, y + height - radius, radius, M_PI / 2, M_PI);
  cairo_close_path(cr);
  cairo_stroke(cr);
}

void RocketRender::text(const int32& x,
                        const int32& y,
                        const std::string& text,
                        const SDL_Color& color)
{
  cairo_t* cr = CairoContext::get_instance()->get_context();
  cairo_set_source_rgba(cr,
                        (float)color.r / 255.0,
                        (float)color.g / 255.0,
                        (float)color.b / 255.0,
                        (float)color.a / 255.0);
  cairo_font_extents_t font_extents;
  cairo_font_extents(cr, &font_extents);
  float32 painter_x = x, painter_y = y + font_extents.height - font_extents.descent;
  // cairo_show_text(cr, text.c_str());
  for (const char& c: text) {
    cairo_move_to(cr, painter_x, painter_y);
    char str[2] = {c, '\0'};
    cairo_show_text(cr, str);
    painter_x += font_extents.max_x_advance;
  }
  // cairo_show_glyphs(cr, nullptr, text.size());
}
