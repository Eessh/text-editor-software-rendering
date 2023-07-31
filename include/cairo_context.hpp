#pragma once

#include <unordered_map>
#include "../cairo-windows-1.17.2/include/cairo-ft.h"
#include "../freetype/freetype/freetype.h"
#include "../freetype/ft2build.h"
#include "cairo.hpp"
#include "window.hpp"

/// @brief CairoContext, wraps cairo context together with freetype
///        font loading and unloading capabilities and font, text metrics.
class CairoContext
{
public:
  CairoContext(const CairoContext& context) = delete;
  CairoContext(CairoContext&& context) = delete;
  CairoContext operator=(const CairoContext& context) = delete;
  CairoContext operator=(CairoContext&& context) = delete;

  /// @brief CairoContext destructor.
  ~CairoContext();

  /// @brief Creates an instance of CairoContext.
  static void create_instance() noexcept;

  /// @brief Gets CairoContext instance.
  /// @return Returns pointer to CairoContext instance.
  [[nodiscard]] static CairoContext* get_instance() noexcept;

  /// @brief Deletes CairoContext instance.
  static void delete_instance() noexcept;

  /// @brief Initializes cairo's context for window's surface.
  /// @param window reference to window.
  void initialize(Window& window) noexcept;

  /// @brief Gets cairo's context.
  /// @return Returns pointer to cairo's context.
  [[nodiscard]] cairo_t* get_context() noexcept;

  /// @brief Reloads cairo's context,
  ///        this should be called when window is resized.
  /// @param window reference to window.
  void reload_context(Window& window) noexcept;

  /// @brief Loads font and assigns it a name.
  /// @param font_name_to_assign name to assign for the loaded font.
  /// @param font_file_path path to font file.
  /// @return Returns false if unable to load font.
  [[nodiscard]] bool load_font(const std::string& font_name_to_assign,
                               const std::string& font_file_path) noexcept;

  /// @brief Sets the font with given assigned name as context's font.
  /// @param font_name assigned font name.
  /// @param font_size font size to set for context.
  /// @return Returns false if no font is assigned for given font name.
  bool set_context_font(const std::string& font_name,
                        const uint8 font_size) noexcept;

  /// @brief Gives the font extents of context's active font.
  /// @return Returns font extents struct (cairo_font_extents_t).
  [[nodiscard]] cairo_font_extents_t get_font_extents() const noexcept;

  /// @brief Gives the text extents for text calculated using context's font.
  /// @param text the text for which extents should be calculated.
  /// @return Returns text extents struct (cairo_text_extents_t).
  [[nodiscard]] cairo_text_extents_t
  get_text_extents(const std::string& text) const noexcept;

private:
  /// @brief Pointer to cairo's context.
  cairo_t* _context;

  /// @brief Instance of freetype library.
  FT_Library _freetype;

  /// @brief Fonts mapped to its name.
  std::unordered_map<std::string, std::pair<FT_Face, cairo_font_face_t*>>
    _font_map;

  /// @brief Context's active font.
  cairo_font_face_t* _active_font_face;

  /// @brief Context's font size.
  uint8 _active_font_size;

  /// @brief Private CairoContext constructor
  CairoContext();

  /// @brief Pointer to CairoContext's instance.
  static CairoContext* _instance;
};