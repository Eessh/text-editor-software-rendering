#include "../include/incremental_render_update.hpp"
#include "../include/config_manager.hpp"
#include "../include/rocket_render.hpp"
#include "../include/utils.hpp"

void ExecuteIncrementalRenderUpdate(
  const IncrementalRenderUpdateCommand& command,
  const float32& scroll_y_offset,
  const cairo_font_extents_t font_extents,
  const Window* window,
  const Buffer& buffer,
  const CppTokenizerCache& tokenizer_cache,
  std::vector<SDL_Rect>& update_rects) noexcept
{
  switch(command.type)
  {
  case IncrementalRenderUpdateType::RENDER_LINE: {
    IncrementalUpdate_RenderLine(command,
                                 scroll_y_offset,
                                 font_extents,
                                 window,
                                 buffer,
                                 tokenizer_cache,
                                 update_rects);
    break;
  }
  case IncrementalRenderUpdateType::RENDER_LINES: {
    IncrementalUpdate_RenderLines(command,
                                  scroll_y_offset,
                                  font_extents,
                                  window,
                                  buffer,
                                  tokenizer_cache,
                                  update_rects);
    break;
  }
  case IncrementalRenderUpdateType::RENDER_LINES_IN_RANGE: {
    IncrementalUpdate_RenderLinesInRange(command,
                                         scroll_y_offset,
                                         font_extents,
                                         window,
                                         buffer,
                                         tokenizer_cache,
                                         update_rects);
    break;
  }
  default:
    break;
  }
}

void IncrementalUpdate_RenderLine(const IncrementalRenderUpdateCommand& command,
                                  const float32& scroll_y_offset,
                                  const cairo_font_extents_t font_extents,
                                  const Window* window,
                                  const Buffer& buffer,
                                  const CppTokenizerCache& tokenizer_cache,
                                  std::vector<SDL_Rect>& update_rects) noexcept
{
  int32 line_y =
    ceil(scroll_y_offset + command.row_start * font_extents.height);
  RocketRender::rectangle_filled(
    0,
    line_y,
    window->width(),
    font_extents.height,
    hexcode_to_SDL_Color(
      ConfigManager::get_instance()->get_config_struct().colorscheme.bg));
  float32 line_numbers_width =
    (std::to_string(buffer.length()).length() + 2) * font_extents.max_x_advance;
  if(ConfigManager::get_instance()->get_config_struct().line_numbers_margin)
  {
    RocketRender::line(
      line_numbers_width,
      0,
      line_numbers_width,
      window->height(),
      hexcode_to_SDL_Color(
        ConfigManager::get_instance()->get_config_struct().colorscheme.gray));
  }
  // drawing line numbers
  std::string number_string = std::move(std::to_string(command.row_start + 1));
  RocketRender::text(
    (std::to_string(buffer.length()).length() - number_string.length() + 1) *
      (font_extents.max_x_advance),
    ceil(scroll_y_offset + font_extents.height * (command.row_start)),
    number_string,
    hexcode_to_SDL_Color(
      ConfigManager::get_instance()->get_config_struct().colorscheme.white));
  // highlight cursor line
  if(buffer.cursor_row() == command.row_start)
  {
    SDL_Color active_line_color = hexcode_to_SDL_Color(
      ConfigManager::get_instance()->get_config_struct().colorscheme.gray);
    active_line_color.a = 32;
    RocketRender::rectangle_filled(line_numbers_width + 1,
                                   line_y,
                                   window->width(),
                                   font_extents.height,
                                   active_line_color);
  }
  // rendering tokens of line
  const std::vector<CppTokenizer::Token>* tokens =
    tokenizer_cache.tokens_for_line(command.row_start);
  render_tokens(line_numbers_width + 1,
                line_y,
                *tokens,
                buffer,
                command.row_start,
                font_extents);
  // drawing selection
  auto selection_for_line_result =
    buffer.selection_slice_for_line(command.row_start);
  if(selection_for_line_result != std::nullopt)
  {
    auto selection = selection_for_line_result.value();
    RocketRender::rectangle_filled(
      line_numbers_width + 1 +
        (selection.first + 1) * font_extents.max_x_advance,
      ceil(scroll_y_offset + line_y),
      (selection.second - selection.first) * font_extents.max_x_advance,
      font_extents.height,
      hexcode_to_SDL_Color(ConfigManager::get_instance()
                             ->get_config_struct()
                             .colorscheme.highlight));
  }
  // drawing cursor
  std::pair<uint32, int32> cursor_coords = buffer.cursor_coords();
  std::string cursor_style =
    ConfigManager::get_instance()->get_config_struct().caret.style;
  RocketRender::rectangle_filled(
    line_numbers_width + 1 +
      font_extents.max_x_advance * (cursor_coords.second + 1),
    ceil(scroll_y_offset + font_extents.height * cursor_coords.first),
    (cursor_style == "ibeam"
       ? ConfigManager::get_instance()->get_config_struct().caret.ibeam_width
       : font_extents.max_x_advance),
    font_extents.height,
    hexcode_to_SDL_Color(
      ConfigManager::get_instance()->get_config_struct().caret.color));
  update_rects.push_back((SDL_Rect){0,
                                    static_cast<int>(line_y),
                                    static_cast<int>(window->width()),
                                    static_cast<int>(font_extents.height)});
}

void IncrementalUpdate_RenderLines(
  const IncrementalRenderUpdateCommand& command,
  const float32& scroll_y_offset,
  const cairo_font_extents_t font_extents,
  const Window* window,
  const Buffer& buffer,
  const CppTokenizerCache& tokenizer_cache,
  std::vector<SDL_Rect>& update_rects) noexcept
{
  IncrementalUpdate_RenderLine(command,
                               scroll_y_offset,
                               font_extents,
                               window,
                               buffer,
                               tokenizer_cache,
                               update_rects);
  IncrementalRenderUpdateCommand command_for_second_line = command;
  command_for_second_line.type = IncrementalRenderUpdateType::RENDER_LINE;
  command_for_second_line.row_start = command.row_end;
  command_for_second_line.row_end = 0;
  IncrementalUpdate_RenderLine(command_for_second_line,
                               scroll_y_offset,
                               font_extents,
                               window,
                               buffer,
                               tokenizer_cache,
                               update_rects);
}

void IncrementalUpdate_RenderLinesInRange(
  const IncrementalRenderUpdateCommand& command,
  const float32& scroll_y_offset,
  const cairo_font_extents_t font_extents,
  const Window* window,
  const Buffer& buffer,
  const CppTokenizerCache& tokenizer_cache,
  std::vector<SDL_Rect>& update_rects) noexcept
{
  IncrementalRenderUpdateCommand command_copy = command;
  command_copy.type = IncrementalRenderUpdateType::RENDER_LINE;
  for(uint32 i = command_copy.row_start; i <= command.row_end; i++)
  {
    command_copy.row_start = i;
    IncrementalUpdate_RenderLine(command_copy,
                                 scroll_y_offset,
                                 font_extents,
                                 window,
                                 buffer,
                                 tokenizer_cache,
                                 update_rects);
  }
}
