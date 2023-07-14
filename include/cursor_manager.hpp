#pragma once

#include "sdl2.hpp"

class CursorManager
{
public:
  CursorManager(const CursorManager& manager) = delete;
  CursorManager(CursorManager&& manager) = delete;
  CursorManager operator=(const CursorManager& manager) = delete;
  CursorManager operator=(CursorManager&& manager) = delete;

  ~CursorManager() noexcept;

  static void create_instance() noexcept;

  static CursorManager* get_instance() noexcept;

  static void delete_insance() noexcept;

  void load_system_cursors() noexcept;

  void set_arrow() const noexcept;

  void set_crosshair() const noexcept;

  void set_double_arrow_left_to_right() const noexcept;

  void set_double_arrow_top_left_to_bottom_right() const noexcept;

  void set_double_arrow_top_right_to_bottom_left() const noexcept;

  void set_double_arrow_top_to_down() const noexcept;

  void set_hand() const noexcept;

  void set_ibeam() const noexcept;

  void set_prohibited() const noexcept;

  void set_small_loading() const noexcept;

  void set_loading() const noexcept;

private:
  SDL_Cursor* _arrow;
  SDL_Cursor* _ibeam;
  SDL_Cursor* _loading;
  SDL_Cursor* _small_loading;
  SDL_Cursor* _crosshair;
  SDL_Cursor* _double_arrow_left_to_right;
  SDL_Cursor* _double_arrow_top_to_down;
  SDL_Cursor* _double_arrow_top_left_to_bottom_right;
  SDL_Cursor* _double_arrow_top_right_to_bottom_left;
  SDL_Cursor* _prohibited;
  SDL_Cursor* _hand;
  SDL_Cursor* _drag;

  CursorManager() noexcept;

  static CursorManager* _instance;
};