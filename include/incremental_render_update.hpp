#pragma once

#include "types.hpp"

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