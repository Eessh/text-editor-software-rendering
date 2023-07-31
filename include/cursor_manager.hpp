#pragma once

#include "sdl2.hpp"

class CursorManager
{
public:
  CursorManager(const CursorManager& manager) = delete;
  CursorManager(CursorManager&& manager) = delete;
  CursorManager operator=(const CursorManager& manager) = delete;
  CursorManager operator=(CursorManager&& manager) = delete;

  /// @brief CursorManager destructor.
  /// @throws No exceptions.
  ~CursorManager() noexcept;

  /// @brief Creates an instance of CursorManager.
  /// @throws No exceptions.
  static void create_instance() noexcept;

  /// @brief Gets CursorManager instance.
  /// @return Returns pointer to CursorManager instance.
  /// @throws No exceptions.
  [[nodiscard]] static CursorManager* get_instance() noexcept;

  /// @brief Deletes CursorManager instance.
  /// @throws No exceptions.
  static void delete_insance() noexcept;

  /// @brief Loads all of system cursors.
  /// @throws No exceptions.
  void load_system_cursors() noexcept;

  /// @brief Sets arrow as active cursor.
  /// @throws No exceptions.
  void set_arrow() const noexcept;

  /// @brief Sets crosshair as active cursor.
  /// @throws No exceptions.
  void set_crosshair() const noexcept;

  /// @brief Sets double-arrow-left-to-right as active cursor.
  /// @throws No exceptions.
  void set_double_arrow_left_to_right() const noexcept;

  /// @brief Sets double-arrow-top-left-to-bottom-right as active cursor.
  /// @throws No exceptions.
  void set_double_arrow_top_left_to_bottom_right() const noexcept;

  /// @brief Sets double-arrow-top-right-to-bottom-left as active cursor.
  /// @throws No exceptions.
  void set_double_arrow_top_right_to_bottom_left() const noexcept;

  /// @brief Sets double-arrow-top-to-down as active cursor.
  /// @throws No exceptions.
  void set_double_arrow_top_to_down() const noexcept;

  /// @brief Sets hand as active cursor.
  /// @throws No exceptions.
  void set_hand() const noexcept;

  /// @brief Sets ibeam as active cursor.
  /// @throws No exceptions.
  void set_ibeam() const noexcept;

  /// @brief Sets prohibited as active cursor.
  /// @throws No exceptions.
  void set_prohibited() const noexcept;

  /// @brief Sets small-loading as active cursor.
  /// @throws No exceptions.
  void set_small_loading() const noexcept;

  /// @brief Sets loading as active cursor.
  /// @throws No exceptions.
  void set_loading() const noexcept;

private:
  /// @brief Arrow cursor.
  SDL_Cursor* _arrow;

  /// @brief I-Beam cursor.
  SDL_Cursor* _ibeam;

  /// @brief Loading cursor.
  SDL_Cursor* _loading;

  /// @brief Small-Loading cursor.
  SDL_Cursor* _small_loading;

  /// @brief Crosshair cursor.
  SDL_Cursor* _crosshair;

  /// @brief Double-Arrow-Left-to-right (<->) cursor.
  SDL_Cursor* _double_arrow_left_to_right;

  /// @brief Double-Arrow-Top-to-Down cursor.
  SDL_Cursor* _double_arrow_top_to_down;

  /// @brief Double-Arrow-TopLeft-to-BottomRight cursor.
  SDL_Cursor* _double_arrow_top_left_to_bottom_right;

  /// @brief Double-Arrow-TopRight-to-BottomLeft cursor.
  SDL_Cursor* _double_arrow_top_right_to_bottom_left;

  /// @brief Prohibited cursor.
  SDL_Cursor* _prohibited;

  /// @brief Hand cursor.
  SDL_Cursor* _hand;

  /// @brief Drag cursor.
  SDL_Cursor* _drag;

  /// @brief Private CursorManager constructor
  /// @throws No exceptions.
  CursorManager() noexcept;

  /// @brief Pointer to CursorManager's instance.
  static CursorManager* _instance;
};