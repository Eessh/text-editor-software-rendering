#include "../include/cursor_manager.hpp"
#include "../include/macros.hpp"

CursorManager* CursorManager::_instance = nullptr;

CursorManager::CursorManager() noexcept
  : _arrow(nullptr)
  , _crosshair(nullptr)
  , _double_arrow_left_to_right(nullptr)
  , _double_arrow_top_left_to_bottom_right(nullptr)
  , _double_arrow_top_right_to_bottom_left(nullptr)
  , _double_arrow_top_to_down(nullptr)
  , _hand(nullptr)
  , _ibeam(nullptr)
  , _loading(nullptr)
  , _prohibited(nullptr)
  , _small_loading(nullptr)
  , _drag(nullptr)
{}

CursorManager::~CursorManager() noexcept
{
  SDL_FreeCursor(_arrow);
  SDL_FreeCursor(_crosshair);
  SDL_FreeCursor(_double_arrow_top_to_down);
  SDL_FreeCursor(_double_arrow_top_right_to_bottom_left);
  SDL_FreeCursor(_double_arrow_top_left_to_bottom_right);
  SDL_FreeCursor(_double_arrow_left_to_right);
  SDL_FreeCursor(_small_loading);
  SDL_FreeCursor(_loading);
  SDL_FreeCursor(_prohibited);
  SDL_FreeCursor(_ibeam);
  SDL_FreeCursor(_hand);
  SDL_FreeCursor(_drag);
}

void CursorManager::create_instance() noexcept
{
  if(_instance)
  {
    ERROR_BOII("CursorManager is already instantiated, use "
               "CursorManager::get_instance()");
    return;
  }

  _instance = new CursorManager();
}

CursorManager* CursorManager::get_instance() noexcept
{
  return _instance;
}

void CursorManager::delete_insance() noexcept
{
  delete _instance;
}

void CursorManager::load_system_cursors() noexcept
{
  INFO_BOII("Loading system cursors...");
  _arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  _ibeam = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
  _loading = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
  _small_loading = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
  _crosshair = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
  _double_arrow_top_left_to_bottom_right =
    SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
  _double_arrow_top_right_to_bottom_left =
    SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
  _double_arrow_left_to_right =
    SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
  _double_arrow_top_to_down = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
  _drag = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
  _prohibited = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
  _hand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
}

void CursorManager::set_arrow() const noexcept
{
  SDL_SetCursor(_arrow);
}

void CursorManager::set_crosshair() const noexcept
{
  SDL_SetCursor(_crosshair);
}

void CursorManager::set_double_arrow_left_to_right() const noexcept
{
  SDL_SetCursor(_double_arrow_left_to_right);
}

void CursorManager::set_double_arrow_top_left_to_bottom_right() const noexcept
{
  SDL_SetCursor(_double_arrow_top_left_to_bottom_right);
}

void CursorManager::set_double_arrow_top_right_to_bottom_left() const noexcept
{
  SDL_SetCursor(_double_arrow_top_right_to_bottom_left);
}

void CursorManager::set_double_arrow_top_to_down() const noexcept
{
  SDL_SetCursor(_double_arrow_top_to_down);
}

void CursorManager::set_hand() const noexcept
{
  SDL_SetCursor(_hand);
}

void CursorManager::set_ibeam() const noexcept
{
  SDL_SetCursor(_ibeam);
}

void CursorManager::set_loading() const noexcept
{
  SDL_SetCursor(_loading);
}

void CursorManager::set_prohibited() const noexcept
{
  SDL_SetCursor(_prohibited);
}

void CursorManager::set_small_loading() const noexcept
{
  SDL_SetCursor(_small_loading);
}