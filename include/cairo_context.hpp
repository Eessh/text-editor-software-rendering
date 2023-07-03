#pragma once

#include <unordered_map>
#include "../cairo-windows-1.17.2/include/cairo-ft.h"
#include "../freetype/freetype/freetype.h"
#include "../freetype/ft2build.h"
#include "cairo.hpp"
#include "window.hpp"

class CairoContext
{
public:
  CairoContext(const CairoContext& context) = delete;
  CairoContext(CairoContext&& context) = delete;
  CairoContext operator=(const CairoContext& context) = delete;
  CairoContext operator=(CairoContext&& context) = delete;

  ~CairoContext();

  /// @brief Creates an instance of CairoContext.
  static void create_instance();

  /// @brief Gets CairoContext instance.
  /// @return Returns CairoContext instance.
  static CairoContext* get_instance();

  /// @brief Deletes CairoContext instance.
  static void delete_instance();

  /// @brief Initializes cairo's context for window's surface.
  /// @param window reference to window.
  void initialize(Window& window);

  /// @brief Gets cairo's context.
  /// @return Returns pointer to cairo's context.
  cairo_t* get_context();

  /// @brief Reloads cairo's context,
  ///        this should be called when window is resized.
  /// @param window reference to window.
  void reload_context(Window& window);

  bool load_font(const std::string& font_name_to_assign,
                 const std::string& font_file_path);

  bool set_context_font(const std::string& font_name, const uint8 font_size);

private:
  /// @brief Pointer to cairo's context.
  cairo_t* _context;

  /// @brief Instance of freetype library.
  FT_Library _freetype;

  /// @brief Fonts mapped to its name.
  std::unordered_map<std::string, cairo_font_face_t*> _font_map;

  /// @brief Private constructor
  CairoContext();

  /// @brief Pointer to CairoContext's instance.
  static CairoContext* _instance;
};