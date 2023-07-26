#pragma once

#include "types.hpp"

enum class IncrementalRenderUpdateType
{
  RENDER_LINE,
  RENDER_LINE_SELECTION,
  RENDER_LINE_SLICE,
  RENDER_LINE_SLICE_SELECTION,
  RENDER_LINES_FROM,
  CLEAR_LINE_SLICE
};

struct IncrementalRenderUpdateCommand
{
  IncrementalRenderUpdateType type;
  uint32 row;
  uint32 token_start, token_end;
  int32 slice_selection_start, slice_selection_end;

  IncrementalRenderUpdateCommand()
    : type(IncrementalRenderUpdateType::RENDER_LINE)
    , row(0)
    , token_start(0)
    , token_end(0)
    , slice_selection_start(-1)
    , slice_selection_end(-1)
  {}
};