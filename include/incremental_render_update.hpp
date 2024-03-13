#pragma once

#include "buffer.hpp"
#include "cairo.hpp"
#include "sdl2.hpp"
#include "types.hpp"
#include "window.hpp"

enum class IncrementalRenderUpdateType
{
  RENDER_LINE,
  RENDER_LINE_SLICE,
  RENDER_LINE_SELECTION,
  RENDER_LINE_SLICE_SELECTION,
  RENDER_LINES,
  RENDER_LINES_IN_RANGE,
  RENDER_CHARACTER,
  RENDER_CURSOR,
};

struct IncrementalRenderUpdateCommand
{
  IncrementalRenderUpdateType type;
  uint32 row_start, row_end;
  int32 slice_start, slice_end;
  int32 slice_selection_start, slice_selection_end;

  IncrementalRenderUpdateCommand()
    : type(IncrementalRenderUpdateType::RENDER_LINE)
    , row_start(0)
    , row_end(0)
    , slice_start(-1)
    , slice_end(-1)
    , slice_selection_start(-1)
    , slice_selection_end(-1)
  {}
};

void ExecuteIncrementalRenderUpdate(
  const IncrementalRenderUpdateCommand& command,
  const float32& scroll_y_offset,
  const cairo_font_extents_t font_extents,
  const Window* window,
  const Buffer& buffer,
  const CppTokenizerCache& tokenizer_cache,
  std::vector<SDL_Rect>& update_rects) noexcept;

void IncrementalUpdate_RenderLine(const IncrementalRenderUpdateCommand& command,
                                  const float32& scroll_y_offset,
                                  const cairo_font_extents_t font_extents,
                                  const Window* window,
                                  const Buffer& buffer,
                                  const CppTokenizerCache& tokenizer_cache,
                                  std::vector<SDL_Rect>& update_rects) noexcept;
void IncrementalUpdate_RenderLines(
  const IncrementalRenderUpdateCommand& command,
  const float32& scroll_y_offset,
  const cairo_font_extents_t font_extents,
  const Window* window,
  const Buffer& buffer,
  const CppTokenizerCache& tokenizer_cache,
  std::vector<SDL_Rect>& update_rects) noexcept;
void IncrementalUpdate_RenderLinesInRange(
  const IncrementalRenderUpdateCommand& command,
  const float32& scroll_y_offset,
  const cairo_font_extents_t font_extents,
  const Window* window,
  const Buffer& buffer,
  const CppTokenizerCache& tokenizer_cache,
  std::vector<SDL_Rect>& update_rects) noexcept;
