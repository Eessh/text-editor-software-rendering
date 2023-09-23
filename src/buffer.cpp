#include "../include/buffer.hpp"
#include <algorithm>
#include <fstream>
#include <string_view>
#include "../include/config_manager.hpp"
#include "../include/macros.hpp"

Buffer::Buffer() noexcept
  : _cursor_row(0)
  , _cursor_col(-1)
  , _cursor_col_target(-1)
  , _has_selection(false)
  , _selection({{0, -1}, {0, -1}})
  , _lines({""})
// , _buffer_incremental_render_update_commands(std::deque<BufferViewUpdateCommand>())
{}

Buffer::Buffer(const std::string& init_string) noexcept
  : _cursor_row(0)
  , _cursor_col(-1)
  , _cursor_col_target(-1)
  , _has_selection(false)
  , _selection({{0, -1}, {0, -1}})
  , _lines({init_string})
// , _buffer_incremental_render_update_commands(std::deque<BufferViewUpdateCommand>())
{}

Buffer::Buffer(const std::vector<std::string>& lines) noexcept
  : _cursor_row(0)
  , _cursor_col(-1)
  , _cursor_col_target(-1)
  , _has_selection(false)
  , _selection({{0, -1}, {0, -1}})
  , _lines(lines)
// , _buffer_incremental_render_update_commands(std::deque<BufferViewUpdateCommand>())
{}

bool Buffer::load_from_file(const std::string& filepath) noexcept
{
  std::ifstream file(filepath);
  if(file.is_open()) [[likely]]
  {
    // setting to defaults
    _cursor_row = 0;
    _cursor_col = -1;
    _cursor_col_target = -1;
    _has_selection = false;
    _selection = {{0, -1}, {0, -1}};
    _lines.clear();
    // _buffer_incremental_render_update_commands.clear();
    _buffer_incremental_render_update_commands.clear();

    while(file.good())
    {
      _lines.emplace_back("");
      std::getline(file, _lines.back());

      // replace all tabs with corresponding amount of spaces
      uint8 tab_width =
        ConfigManager::get_instance()->get_config_struct().tab_width;
      auto it = _lines.back().find('\t');
      while(it != std::string::npos)
      {
        _lines.back().replace(it, 1, std::string(tab_width, ' '));
        it = _lines.back().find('\t');
      }
    }
    return true;
  }
  else [[unlikely]]
  {
    ERROR_BOII("Unable to open file: %s", filepath.c_str());
    return false;
  }
}

uint32 Buffer::length() const noexcept
{
  return _lines.size();
}

std::optional<uint32>
Buffer::line_length(const uint32& line_index) const noexcept
{
  if(line_index >= _lines.size()) [[unlikely]]
  {
    ERROR_BOII("Accessing line length with line_index out of bounds!");
    return std::nullopt;
  }
  else [[likely]]
  {
    return _lines[line_index].size();
  }
}

const std::vector<std::string>& Buffer::lines() const noexcept
{
  return _lines;
}

std::optional<const std::reference_wrapper<std::string>>
Buffer::line(const uint32& line_index) const noexcept
{
  if(line_index >= _lines.size()) [[unlikely]]
  {
    ERROR_BOII("Accessing line with line_index: %ld out of bounds!",
               line_index);
    return std::nullopt;
  }
  else [[likely]]
  {
    return const_cast<std::string&>(_lines[line_index]);
  }
}

std::optional<std::string> Buffer::line_with_spaces_converted_to_tabs(
  const uint32& line_index) const noexcept
{
  if(line_index >= _lines.size()) [[unlikely]]
  {
    ERROR_BOII("Accessing line with line_index: %ld out of bounds!",
               line_index);
    return std::nullopt;
  }
  else [[likely]]
  {
    std::string line(_lines[line_index]);
    _convert_leading_spaces_to_indentation_tabs(line);
    return line;
  }
}

std::pair<uint32, int32> Buffer::cursor_coords() const noexcept
{
  return std::make_pair(_cursor_row, _cursor_col);
}

const uint32& Buffer::cursor_row() const noexcept
{
  return _cursor_row;
}

uint32& Buffer::cursor_row() noexcept
{
  return _cursor_row;
}

void Buffer::set_cursor_row(const uint32& row) noexcept
{
  IncrementalRenderUpdateCommand cmd;
  cmd.type = IncrementalRenderUpdateType::RENDER_LINES;
  cmd.row_start = _cursor_row;
  cmd.row_end = row;
  _buffer_incremental_render_update_commands.push_back(cmd);
  DEBUG_BOII("set row: %ld", row);
  _cursor_row = row;
}

const int32& Buffer::cursor_column() const noexcept
{
  return _cursor_col;
}

int32& Buffer::cursor_column() noexcept
{
  return _cursor_col;
}

void Buffer::set_cursor_column(const int32& column) noexcept
{
  IncrementalRenderUpdateCommand cmd;
  cmd.type = IncrementalRenderUpdateType::RENDER_LINE;
  cmd.row_start = _cursor_row;
  _buffer_incremental_render_update_commands.push_back(cmd);
  _cursor_col = column;
}

void Buffer::set_cursor_column_target(const int32& column_target) noexcept
{
  _cursor_col_target = column_target;
}

bool Buffer::has_selection() const noexcept
{
  return _has_selection;
}

void Buffer::clear_selection() noexcept
{
  if(!_has_selection)
  {
    return;
  }

  auto selection = this->selection().value();
  IncrementalRenderUpdateCommand cmd;
  cmd.type = IncrementalRenderUpdateType::RENDER_LINES_IN_RANGE;
  cmd.row_start = selection.first.first;
  cmd.row_end = selection.second.first;
  _buffer_incremental_render_update_commands.push_back(cmd);
  _has_selection = false;
}

std::optional<std::pair<std::pair<uint32, int32>, std::pair<uint32, int32>>>
Buffer::selection() const noexcept
{
  if(!_has_selection) [[unlikely]]
  {
    ERROR_BOII("Called GET selection, when buffer has no selection!");
    return std::nullopt;
  }

  // sort selection start, end
  if(_selection.first.first < _selection.second.first)
  {
    return _selection;
  }
  if(_selection.first.first == _selection.second.first &&
     _selection.first.second < _selection.second.second)
  {
    return _selection;
  }

  return std::make_pair(_selection.second, _selection.first);
}

std::optional<std::pair<int32, int32>>
Buffer::selection_slice_for_line(const uint32& line_index) const noexcept
{
  if(!_has_selection)
  {
    return std::nullopt;
  }

  auto selection = this->selection().value();
  if(line_index < selection.first.first || line_index > selection.second.first)
  {
    return std::nullopt;
  }

  if(selection.first.first == selection.second.first)
  {
    // selection in one line, and line is line_index
    return std::make_pair(selection.first.second, selection.second.second);
  }

  if(line_index == selection.first.first)
  {
    return std::make_pair(selection.first.second,
                          static_cast<int32>(_lines[line_index].size() - 1));
  }

  if(line_index == selection.second.first)
  {
    return std::make_pair(-1, selection.second.second);
  }

  return std::make_pair(-1, static_cast<int32>(_lines[line_index].size() - 1));
}

void Buffer::set_selection_start_coordinate(
  const std::pair<uint32, int32>& coordinate) noexcept
{
  _has_selection = false;

  /// FIXME: check the row, columns before assigning the coodinate u lazy shit.

  _selection.first = coordinate;
}

void Buffer::set_selection_end_coordinate(
  const std::pair<uint32, int32>& coordinate) noexcept
{
  if(_selection.first == coordinate)
  {
    _has_selection = false;
    return;
  }

  _has_selection = true;

  /// FIXME: check the row, columns before assigning the coodinate u lazy shit.

  _selection.second = coordinate;
}

void Buffer::execute_cursor_command(const BufferCursorCommand& command) noexcept
{
  switch(command)
  {
  case BufferCursorCommand::MOVE_LEFT: {
    if(_has_selection)
    {
      auto selection = this->selection().value();
      _cursor_row = selection.first.first;
      _cursor_col = selection.first.second;
      _cursor_col_target = _cursor_col;
      _has_selection = false;
      IncrementalRenderUpdateCommand cmd;
      cmd.type = IncrementalRenderUpdateType::RENDER_CURSOR;
      _buffer_incremental_render_update_commands.push_back(cmd);
      cmd.type = IncrementalRenderUpdateType::RENDER_CHARACTER;
      cmd.row_start = selection.first.first;
      cmd.row_end = selection.second.first;
      return;
    }

    this->_base_move_cursor_left();

    break;
  }
  case BufferCursorCommand::MOVE_RIGHT: {
    if(_has_selection)
    {
      auto selection = this->selection().value();
      _cursor_row = selection.second.first;
      _cursor_col = selection.second.second;
      _cursor_col_target = _cursor_col;
      _has_selection = false;
      IncrementalRenderUpdateCommand cmd;
      cmd.type = IncrementalRenderUpdateType::RENDER_LINES_IN_RANGE;
      cmd.row_start = selection.first.first;
      cmd.row_end = selection.second.first;
      _buffer_incremental_render_update_commands.push_back(cmd);
      return;
    }

    this->_base_move_cursor_right();

    break;
  }
  case BufferCursorCommand::MOVE_UP: {
    if(_has_selection)
    {
      auto selection = this->selection().value();
      _cursor_row = selection.first.first;
      _cursor_col = selection.first.second;
      _cursor_col_target = _cursor_col;
      _has_selection = false;
      IncrementalRenderUpdateCommand cmd;
      cmd.type = IncrementalRenderUpdateType::RENDER_LINES_IN_RANGE;
      cmd.row_end = selection.second.first;
      if(_cursor_row == 0)
      {
        cmd.row_start = selection.first.first;
        _buffer_incremental_render_update_commands.push_back(cmd);
        return;
      }
      --_cursor_row;
      cmd.row_start = _cursor_row;
      if(static_cast<int32>(_lines[_cursor_row].size() - 1) <
         _cursor_col_target)
      {
        _cursor_col = _lines[_cursor_row].size() - 1;
      }
      _buffer_incremental_render_update_commands.push_back(cmd);
      return;
    }

    this->_base_move_cursor_up();

    break;
  }
  case BufferCursorCommand::MOVE_DOWN: {
    if(_has_selection)
    {
      auto selection = this->selection().value();
      _cursor_row = selection.second.first;
      _cursor_col = selection.second.second;
      _cursor_col_target = _cursor_col;
      _has_selection = false;
      IncrementalRenderUpdateCommand cmd;
      cmd.type = IncrementalRenderUpdateType::RENDER_LINES_IN_RANGE;
      cmd.row_start = selection.first.first;
      if(_cursor_row == _lines.size() - 1)
      {
        cmd.row_end = selection.second.first;
        _buffer_incremental_render_update_commands.push_back(cmd);
        return;
      }
      ++_cursor_row;
      cmd.row_end = _cursor_row;
      if(static_cast<int32>(_lines[_cursor_row].size() - 1) <
         _cursor_col_target)
      {
        _cursor_col = _lines[_cursor_row].size() - 1;
      }
      _buffer_incremental_render_update_commands.push_back(cmd);
      return;
    }

    this->_base_move_cursor_down();

    break;
  }
  case BufferCursorCommand::MOVE_TO_PREVIOUS_WORD_START: {
    if(_has_selection)
    {
      // move to selection starting and
      // move to previous word start
      auto selection_ = this->selection().value();
      _cursor_row = selection_.first.first;
      _cursor_col = selection_.first.second;
      _has_selection = false;

      /// TODO: Add incremental render update command for clearing selection
    }

    this->_base_move_cursor_to_previous_word_start();
    break;
  }
  case BufferCursorCommand::MOVE_TO_NEXT_WORD_END: {
    if(_has_selection)
    {
      // move to selection ending and
      // move to next word end
      auto selection_ = this->selection().value();
      _cursor_row = selection_.second.first;
      _cursor_col = selection_.second.second;
      _has_selection = false;

      /// TODO: Add incremental render update command for clearing selection
    }

    this->_base_move_cursor_to_next_word_end();
    break;
  }
  default:
    break;
  }
}

void Buffer::execute_selection_command(
  const BufferSelectionCommand& command) noexcept
{
  switch(command)
  {
  case BufferSelectionCommand::MOVE_LEFT: {
    if(!_has_selection)
    {
      // set current cursor position as selection start
      // move cursor left, make it end positon
      _has_selection = true;
      _selection.first.first = _cursor_row;
      _selection.first.second = _cursor_col;

      // this also handles the view update command
      if(!this->_base_move_cursor_left()) [[unlikely]]
      {
        // cursor hasn't moved
        _has_selection = false;
        return;
      }

      _selection.second.first = _cursor_row;
      _selection.second.second = _cursor_col;
      return;
    }

    // this also handles the view update command
    this->_base_move_cursor_left();
    _selection.second.first = _cursor_row;
    _selection.second.second = _cursor_col;
    break;
  }
  case BufferSelectionCommand::MOVE_RIGHT: {
    if(!_has_selection)
    {
      // set current cursor position as selection start
      // move cursor right, make it end positon
      _has_selection = true;
      _selection.first.first = _cursor_row;
      _selection.first.second = _cursor_col;

      // this also handles the view update command
      if(!this->_base_move_cursor_right()) [[unlikely]]
      {
        // cursor hasn't moved
        _has_selection = false;
        return;
      }

      _selection.second.first = _cursor_row;
      _selection.second.second = _cursor_col;
      return;
    }

    // this also handles the view update command
    this->_base_move_cursor_right();
    _selection.second.first = _cursor_row;
    _selection.second.second = _cursor_col;
    break;
  }
  case BufferSelectionCommand::MOVE_UP: {
    if(!_has_selection)
    {
      // set current cursor position as selection start
      // move cursor up, make it end positon
      _has_selection = true;
      _selection.first.first = _cursor_row;
      _selection.first.second = _cursor_col;

      // this also handles the view update command
      if(!this->_base_move_cursor_up()) [[unlikely]]
      {
        // cursor hasn't moved
        _has_selection = false;
        return;
      }

      _selection.second.first = _cursor_row;
      _selection.second.second = _cursor_col;
      return;
    }

    // this also handles the view update command
    this->_base_move_cursor_up();
    _selection.second.first = _cursor_row;
    _selection.second.second = _cursor_col;
    break;
  }
  case BufferSelectionCommand::MOVE_DOWN: {
    if(!_has_selection)
    {
      // set current cursor position as selection start
      // move cursor down, make it end positon
      _has_selection = true;
      _selection.first.first = _cursor_row;
      _selection.first.second = _cursor_col;

      // this also handles the view update command
      if(!this->_base_move_cursor_down()) [[unlikely]]
      {
        // cursor hasn't moved
        _has_selection = false;
        return;
      }

      _selection.second.first = _cursor_row;
      _selection.second.second = _cursor_col;
      return;
    }

    // this also handles the view update command
    this->_base_move_cursor_down();
    _selection.second.first = _cursor_row;
    _selection.second.second = _cursor_col;
    break;
  }
  case BufferSelectionCommand::EXTEND_TO_PREVIOUS_WORD_START: {
    if(!_has_selection)
    {
      _has_selection = true;
      _selection.first.first = _cursor_row;
      _selection.first.second = _cursor_col;

      if(!this->_base_move_cursor_to_previous_word_start())
      {
        _has_selection = false;
      }

      _selection.second.first = _cursor_row;
      _selection.second.second = _cursor_col;
      return;
    }

    if(this->_base_move_cursor_to_previous_word_start())
    {
      _selection.second.first = _cursor_row;
      _selection.second.second = _cursor_col;
    }
    break;
  }
  case BufferSelectionCommand::EXTENT_TO_NEXT_WORD_END: {
    if(!_has_selection)
    {
      _has_selection = true;
      _selection.first.first = _cursor_row;
      _selection.first.second = _cursor_col;

      if(!this->_base_move_cursor_to_next_word_end())
      {
        _has_selection = false;
      }

      _selection.second.first = _cursor_row;
      _selection.second.second = _cursor_col;
      return;
    }

    if(this->_base_move_cursor_to_next_word_end())
    {
      _selection.second.first = _cursor_row;
      _selection.second.second = _cursor_col;
    }
    break;
  }
  case BufferSelectionCommand::SELECT_WORD: {
    std::string word_delimiters = " \t\n/\\()\"':,.;<>~!@#$%^&*|+=[]{}`?-";
    int16 left = 0, right = 1;
    while(_cursor_col - left != -1)
    {
      if(word_delimiters.find(_lines[_cursor_row][_cursor_col - left]) !=
         std::string::npos)
      {
        break;
      }
      left++;
    }
    while(_cursor_col + right < _lines[_cursor_row].size())
    {
      if(word_delimiters.find(_lines[_cursor_row][_cursor_col + right]) !=
         std::string::npos)
      {
        break;
      }
      right++;
    }
    if(left == 0 && right == 0)
    {
      return;
    }

    _has_selection = true;
    _selection.first.first = _cursor_row;
    _selection.first.second = _cursor_col - left;
    _selection.second.first = _cursor_row;
    _selection.second.second = _cursor_col + right - 1;
    _cursor_col += right - 1;
    IncrementalRenderUpdateCommand cmd;
    cmd.type = IncrementalRenderUpdateType::RENDER_LINE;
    cmd.row_start = _cursor_row;
    _buffer_incremental_render_update_commands.push_back(cmd);
    break;
  }
  case BufferSelectionCommand::SELECT_LINE: {
    _has_selection = true;
    _selection.first.first = _cursor_row;
    _selection.first.second = -1;
    _selected_line = _cursor_row;
    if(_cursor_row == _lines.size() - 1)
    {
      // ending line
      _selection.second.first = _cursor_row;
      _selection.second.second = _lines[_cursor_row].size() - 1;
      _cursor_col = _lines[_cursor_row].size() - 1;
      IncrementalRenderUpdateCommand cmd;
      cmd.type = IncrementalRenderUpdateType::RENDER_LINE;
      cmd.row_start = _cursor_row;
      _buffer_incremental_render_update_commands.push_back(cmd);
    }
    else
    {
      _selection.second.first = _cursor_row + 1;
      _selection.second.second = -1;
      _cursor_col = -1;
      IncrementalRenderUpdateCommand cmd;
      cmd.type = IncrementalRenderUpdateType::RENDER_LINES_IN_RANGE;
      cmd.row_start = _cursor_row;
      _cursor_row += 1;
      cmd.row_end = _cursor_row;
      _buffer_incremental_render_update_commands.push_back(cmd);
    }
    break;
  }
  default:
    break;
  }
}

void Buffer::extend_line_selection_to_line(const uint32& line_index) noexcept
{
  if(!_has_selection)
  {
    this->execute_selection_command(BufferSelectionCommand::SELECT_LINE);
    return;
  }

  if(line_index < _selected_line)
  {
    if(_selection.second.first > _selection.first.first)
    {
      // currently selected only one line
      std::swap(_selection.first.first, _selection.second.first);
      std::swap(_selection.first.second, _selection.second.second);
    }

    _selection.second.first = line_index;
    _selection.second.second = -1;

    _cursor_row = line_index;
    _cursor_col = -1;
    return;
  }

  if(line_index == _selected_line)
  {
    _selection.first.first = line_index;
    _selection.first.second = -1;
    if(line_index == _lines.size())
    {
      _selection.second.first = line_index;
      _selection.second.second = _lines[line_index].size() - 1;

      _cursor_row = line_index;
      _cursor_col = _lines[line_index].size() - 1;
    }
    else
    {
      _selection.second.first = line_index + 1;
      _selection.second.second = -1;

      _cursor_row = line_index + 1;
      _cursor_col = -1;
    }
    return;
  }

  if(line_index == _lines.size() - 1)
  {
    _selection.second.first = line_index;
    _selection.second.second = _lines[line_index].size() - 1;

    _cursor_row = line_index;
    _cursor_col = _lines[line_index].size() - 1;
    return;
  }

  _selection.second.first = line_index + 1;
  _selection.second.second = -1;

  _cursor_row = line_index + 1;
  _cursor_col = -1;
}

bool Buffer::process_backspace() noexcept
{
  if(_has_selection)
  {
    this->_delete_selection();
    return true;
  }

  if(_cursor_col == -1 && _cursor_row == 0)
  {
    return false;
  }

  if(_cursor_col != -1)
  {
    int32 leading_spaces_count = this->_line_leading_spaces_count(_cursor_row);
    int32 tab_width =
      ConfigManager::get_instance()->get_config_struct().tab_width;

    if(_cursor_col < leading_spaces_count && (_cursor_col + 1) % tab_width == 0)
    {
      // delete tab width amount of spaces
      _lines[_cursor_row].erase(0, tab_width);
      _cursor_col -= tab_width;
    }
    else if(this->_cursor_between_brackets())
    {
      // auto deleting closing bracket
      _lines[_cursor_row].erase(_cursor_col, 2);
      _cursor_col -= 1;
    }
    else
    {
      // remove character before cursor
      _lines[_cursor_row].erase(_cursor_col, 1);
      _cursor_col -= 1;
    }

    {
      IncrementalRenderUpdateCommand cmd;
      cmd.type = IncrementalRenderUpdateType::RENDER_LINE;
      cmd.row_start = _cursor_row;
      _buffer_incremental_render_update_commands.emplace_back(cmd);
    }
    {
      TokenCacheUpdateCommand cmd;
      cmd.type = TokenCacheUpdateCommandType::RETOKENIZE_LINE;
      cmd.row = _cursor_row;
      _token_cache_update_commands_queue.emplace_back(cmd);
    }
    return true;
  }

  // append the contents of this string to above line
  _cursor_col = _lines[_cursor_row - 1].size() - 1;
  _lines[_cursor_row - 1].append(_lines[_cursor_row]);
  {
    TokenCacheUpdateCommand cmd;
    cmd.type = TokenCacheUpdateCommandType::DELETE_LINE_CACHE;
    cmd.row = _cursor_row;
    _token_cache_update_commands_queue.emplace_back(cmd);
    cmd.type = TokenCacheUpdateCommandType::RETOKENIZE_LINE;
    cmd.row = _cursor_row - 1;
    _token_cache_update_commands_queue.emplace_back(cmd);
  }
  _lines.erase(_lines.begin() + _cursor_row);
  _cursor_row -= 1;
  {
    IncrementalRenderUpdateCommand cmd;
    cmd.type = IncrementalRenderUpdateType::RENDER_LINES_IN_RANGE;
    cmd.row_start = _cursor_row;
    cmd.row_end = _lines.size() - 1;
    _buffer_incremental_render_update_commands.push_back(cmd);
  }
  return true;
}

void Buffer::process_enter() noexcept
{
  if(_has_selection)
  {
    this->_delete_selection();
  }

  uint32 leading_spaces = this->_line_leading_spaces_count(_cursor_row);
  uint8 tab_width =
    ConfigManager::get_instance()->get_config_struct().tab_width;

  if(this->_cursor_between_brackets())
  {
    // insert new line after this line
    // insert leading space of this line + extra indent into new line
    // insert another new line, with same amount if leading spaces as this line
    // and append contents of this line after the cursor to another new line
    _lines.insert(_lines.begin() + _cursor_row + 1,
                  std::string(leading_spaces + tab_width, ' '));
    _lines.insert(_lines.begin() + _cursor_row + 2,
                  std::string(leading_spaces, ' '));
    _lines[_cursor_row + 2].append(_lines[_cursor_row].substr(_cursor_col + 1));
    _lines[_cursor_row].erase(_cursor_col + 1);

    // updating token cache
    TokenCacheUpdateCommand cmd;
    cmd.type = TokenCacheUpdateCommandType::RETOKENIZE_LINE;
    cmd.row = _cursor_row;
    _token_cache_update_commands_queue.emplace_back(cmd);
    cmd.type = TokenCacheUpdateCommandType::INSERT_NEW_LINE_CACHE_AND_TOKENIZE;
    _token_cache_update_commands_queue.emplace_back(cmd);
    cmd.row = _cursor_row + 1;
    _token_cache_update_commands_queue.emplace_back(cmd);

    // updating cursor position
    _cursor_row += 1;
    _cursor_col = leading_spaces + tab_width - 1;
    return;
  }

  if(this->_cursor_at_bracket())
  {
    // insert new line after this line
    // insert leading space of this line + extra indent into new line
    // and append contents of this line after the cursor to new line
    _lines.insert(_lines.begin() + _cursor_row + 1,
                  std::string(leading_spaces + tab_width, ' '));
    _lines[_cursor_row + 1].append(_lines[_cursor_row].substr(_cursor_col + 1));
    _lines[_cursor_row].erase(_cursor_col + 1);

    // updating cursor column
    _cursor_col = leading_spaces + tab_width - 1;
  }
  else
  {
    // insert new line after this line
    // insert leading spaces of this line into new line
    // and append contents of this line after the cursor to new line
    _lines.insert(_lines.begin() + _cursor_row + 1,
                  std::string(leading_spaces, ' '));
    _lines[_cursor_row + 1].append(_lines[_cursor_row].substr(_cursor_col + 1));
    _lines[_cursor_row].erase(_cursor_col + 1);

    // updating cursor column
    _cursor_col = leading_spaces - 1;
  }

  // updating token cache
  {
    TokenCacheUpdateCommand cmd;
    cmd.type = TokenCacheUpdateCommandType::RETOKENIZE_LINE;
    cmd.row = _cursor_row;
    _token_cache_update_commands_queue.emplace_back(cmd);
    cmd.type = TokenCacheUpdateCommandType::INSERT_NEW_LINE_CACHE_AND_TOKENIZE;
    _token_cache_update_commands_queue.emplace_back(cmd);
  }
  {
    IncrementalRenderUpdateCommand cmd;
    cmd.type = IncrementalRenderUpdateType::RENDER_LINES_IN_RANGE;
    cmd.row_start = _cursor_row;
    _cursor_row += 1;
    cmd.row_end = _lines.size() - 1;
    _buffer_incremental_render_update_commands.push_back(cmd);
  }
}

void Buffer::insert_string(const std::string& str) noexcept
{
  if(_has_selection)
  {
    if(str == "(" || str == "[" || str == "{" || str == "\"" || str == "'")
    {
      if(str == "(")
      {
        this->_wrap_selection_with_character('(', ')');
      }
      else if(str == "[")
      {
        this->_wrap_selection_with_character('[', ']');
      }
      else if(str == "{")
      {
        this->_wrap_selection_with_character('{', '}');
      }
      else if(str == "\"")
      {
        this->_wrap_selection_with_character('"', '"');
      }
      else
      {
        this->_wrap_selection_with_character('\'', '\'');
      }

      // update token cache
      {
        auto sel = this->selection().value();

        // for inline selection, only one re-tokenization is required
        TokenCacheUpdateCommand cmd;
        cmd.type = TokenCacheUpdateCommandType::RETOKENIZE_LINE;
        cmd.row = sel.first.first;
        _token_cache_update_commands_queue.emplace_back(cmd);

        // for multiline selection
        // re-tokenization of selection ending line is also needed
        if(sel.first.first != sel.second.first)
        {
          cmd.row = sel.second.first;
          _token_cache_update_commands_queue.emplace_back(cmd);
        }
      }
      return;
    }
    else
    {
      this->_delete_selection();
    }
  }

  // auto closing open brackets
  if(str == "(")
  {
    std::string str_to_insert("()");
    _lines[_cursor_row].insert(_cursor_col + 1, str_to_insert);
    _cursor_col += 1;
  }
  else if(str == "[")
  {
    std::string str_to_insert("[]");
    _lines[_cursor_row].insert(_cursor_col + 1, str_to_insert);
    _cursor_col += 1;
  }
  else if(str == "{")
  {
    std::string str_to_insert("{}");
    _lines[_cursor_row].insert(_cursor_col + 1, str_to_insert);
    _cursor_col += 1;
  }
  else if(str == "\"")
  {
    std::string str_to_insert("\"\"");
    _lines[_cursor_row].insert(_cursor_col + 1, str_to_insert);
    _cursor_col += 1;
  }
  else if(str == "'")
  {
    std::string str_to_insert("''");
    _lines[_cursor_row].insert(_cursor_col + 1, str_to_insert);
    _cursor_col += 1;
  }
  // else inserting string normally
  else
  {
    _lines[_cursor_row].insert(_cursor_col + 1, str);
    _cursor_col += str.size();
  }

  {
    IncrementalRenderUpdateCommand cmd;
    cmd.type = IncrementalRenderUpdateType::RENDER_LINE;
    cmd.row_start = _cursor_row;
    _buffer_incremental_render_update_commands.push_back(cmd);
  }
  {
    TokenCacheUpdateCommand cmd;
    cmd.type = TokenCacheUpdateCommandType::RETOKENIZE_LINE;
    cmd.row = _cursor_row;
    _token_cache_update_commands_queue.emplace_back(cmd);
  }
}

// std::optional<BufferViewUpdateCommand>
// Buffer::get_next_view_update_command() noexcept
// {
//   if(_buffer_incremental_render_update_commands.empty())
//   {
//     return std::nullopt;
//   }

//   BufferViewUpdateCommand command = _buffer_incremental_render_update_commands.front();
//   _buffer_incremental_render_update_commands.pop_front();
//   return command;
// }

void Buffer::remove_most_recent_view_update_command() noexcept
{
  if(_buffer_incremental_render_update_commands.empty())
  {
    return;
  }

  _buffer_incremental_render_update_commands.pop_back();
}

std::optional<TokenCacheUpdateCommand>
Buffer::get_next_token_cache_update_command() noexcept
{
  if(_token_cache_update_commands_queue.empty())
  {
    return std::nullopt;
  }

  TokenCacheUpdateCommand cmd = _token_cache_update_commands_queue.front();
  _token_cache_update_commands_queue.pop_front();
  return cmd;
}

bool Buffer::_base_move_cursor_left() noexcept
{
  if(_cursor_col > -1) [[likely]]
  {
    _cursor_col--;
    _cursor_col_target = _cursor_col;
    IncrementalRenderUpdateCommand cmd;
    cmd.type = IncrementalRenderUpdateType::RENDER_LINE;
    cmd.row_start = _cursor_row;
    _buffer_incremental_render_update_commands.push_back(cmd);
    return true;
  }

  if(_cursor_row == 0)
  {
    return false;
  }

  IncrementalRenderUpdateCommand cmd;
  cmd.type = IncrementalRenderUpdateType::RENDER_LINES_IN_RANGE;
  cmd.row_start = _cursor_row;
  --_cursor_row;
  _cursor_col = _lines[_cursor_row].size() - 1;
  _cursor_col_target = _cursor_col;
  cmd.row_end = _cursor_row;
  _buffer_incremental_render_update_commands.push_back(cmd);

  return true;
}

bool Buffer::_base_move_cursor_right() noexcept
{
  if(_cursor_col < static_cast<int32>(_lines[_cursor_row].size() - 1))
    [[likely]]
  {
    _cursor_col++;
    _cursor_col_target = _cursor_col;
    IncrementalRenderUpdateCommand cmd;
    cmd.type = IncrementalRenderUpdateType::RENDER_LINE;
    cmd.row_start = _cursor_row;
    _buffer_incremental_render_update_commands.push_back(cmd);
    return true;
  }

  if(_cursor_row == _lines.size() - 1)
  {
    return false;
  }

  IncrementalRenderUpdateCommand cmd;
  cmd.type = IncrementalRenderUpdateType::RENDER_LINES_IN_RANGE;
  cmd.row_start = _cursor_row;
  ++_cursor_row;
  _cursor_col = -1;
  _cursor_col_target = _cursor_col;
  cmd.row_end = _cursor_row;
  _buffer_incremental_render_update_commands.push_back(cmd);

  return true;
}

bool Buffer::_base_move_cursor_up() noexcept
{
  if(_cursor_row == 0)
  {
    return false;
  }

  IncrementalRenderUpdateCommand cmd;
  cmd.type = IncrementalRenderUpdateType::RENDER_LINES_IN_RANGE;
  cmd.row_start = _cursor_row;
  --_cursor_row;
  if(static_cast<int32>(_lines[_cursor_row].size() - 1) < _cursor_col_target)
  {
    _cursor_col = _lines[_cursor_row].size() - 1;
  }
  else
  {
    _cursor_col = _cursor_col_target;
  }
  cmd.row_end = _cursor_row;
  _buffer_incremental_render_update_commands.push_back(cmd);

  return true;
}

bool Buffer::_base_move_cursor_down() noexcept
{
  if(_cursor_row == _lines.size() - 1)
  {
    return false;
  }

  IncrementalRenderUpdateCommand cmd;
  cmd.type = IncrementalRenderUpdateType::RENDER_LINES_IN_RANGE;
  cmd.row_start = _cursor_row;
  ++_cursor_row;
  if(static_cast<int32>(_lines[_cursor_row].size() - 1) < _cursor_col_target)
  {
    _cursor_col = _lines[_cursor_row].size() - 1;
  }
  else
  {
    _cursor_col = _cursor_col_target;
  }
  cmd.row_end = _cursor_row;
  _buffer_incremental_render_update_commands.push_back(cmd);

  return true;
}

bool Buffer::_base_move_cursor_to_previous_word_start() noexcept
{
  const std::string_view word_separators =
    ConfigManager::get_instance()->get_config_struct().word_separators;

  int32 row = _cursor_row, col = _cursor_col;
  bool found_word = false;
  while(1)
  {
    if(col == -1)
    {
      if(row == 0)
      {
        return false;
      }
      row -= 1;
      col = _lines[row].size() - 1;
    }
    if(col != -1 && word_separators.find(_lines[row][col]) == std::string::npos)
    {
      found_word = true;
    }
    else
    {
      if(found_word)
      {
        _cursor_row = row;
        _cursor_col = col;
        return true;
      }

      _cursor_row = row;
      _cursor_col = col - 1;
      return true;
    }
    col -= 1;
  }

  return false;
}

bool Buffer::_base_move_cursor_to_next_word_end() noexcept
{
  const std::string_view word_separators =
    ConfigManager::get_instance()->get_config_struct().word_separators;

  int32 row = _cursor_row, col = _cursor_col;
  bool found_word = false;
  while(1)
  {
    if(col == _lines[row].size())
    {
      if(row == _lines.size() - 1)
      {
        return false;
      }
      row += 1;
      col = -1;
    }
    if(col != -1 && word_separators.find(_lines[row][col]) == std::string::npos)
    {
      if(row != _cursor_row || col != _cursor_col)
      {
        found_word = true;
      }
      // cursor maybe at just the end of the word
    }
    else
    {
      if(found_word)
      {
        _cursor_row = row;
        _cursor_col = col - 1;
        return true;
      }

      if(row != _cursor_row || col != _cursor_col)
      {
        _cursor_row = row;
        _cursor_col = col;
        return true;
      }
    }
    col += 1;
  }

  return false;
}

void Buffer::_delete_selection() noexcept
{
  if(!_has_selection)
  {
    return;
  }

  auto selection = this->selection().value();
  if(selection.first.first == selection.second.first)
  {
    // deletion happens in same line
    _lines[selection.first.first].erase(selection.first.second + 1,
                                        selection.second.second -
                                          selection.first.second);
    {
      IncrementalRenderUpdateCommand cmd;
      cmd.type = IncrementalRenderUpdateType::RENDER_LINE;
      cmd.row_start = _cursor_row;
      _buffer_incremental_render_update_commands.push_back(cmd);
    }
    {
      TokenCacheUpdateCommand cmd;
      cmd.type = TokenCacheUpdateCommandType::RETOKENIZE_LINE;
      cmd.row = _cursor_row;
      _token_cache_update_commands_queue.emplace_back(cmd);
    }
  }
  else
  {
    {
      IncrementalRenderUpdateCommand cmd;
      cmd.type = IncrementalRenderUpdateType::RENDER_LINES_IN_RANGE;
      cmd.row_start = selection.first.first;
      cmd.row_end = _lines.size() - 1;
      _buffer_incremental_render_update_commands.push_back(cmd);
    }

    _lines[selection.first.first].erase(selection.first.second + 1);
    _lines[selection.second.first].erase(0, selection.second.second + 1);
    _lines[selection.first.first].append(_lines[selection.second.first]);
    // deleting lines btw selection start end, which are fully selected
    _lines.erase(_lines.begin() + selection.first.first + 1,
                 _lines.begin() + selection.second.first + 1);

    {
      TokenCacheUpdateCommand cmd;
      cmd.type = TokenCacheUpdateCommandType::DELETE_LINES_CACHE;
      cmd.start_row = selection.first.first + 1;
      cmd.end_row = selection.second.first;
      _token_cache_update_commands_queue.emplace_back(cmd);
      cmd.type = TokenCacheUpdateCommandType::RETOKENIZE_LINE;
      cmd.row = selection.first.first;
      _token_cache_update_commands_queue.emplace_back(cmd);
    }
  }
  _cursor_row = selection.first.first;
  _cursor_col = selection.first.second;
  _has_selection = false;
}

uint32
Buffer::_line_leading_spaces_count(const uint32& line_index) const noexcept
{
  uint32 spaces_count = 0;
  while(spaces_count < _lines[line_index].size())
  {
    if(_lines[line_index][spaces_count] == ' ')
    {
      spaces_count++;
      continue;
    }
    break;
  }

  return spaces_count;
}

void Buffer::_convert_leading_spaces_to_indentation_tabs(
  std::string& str) const noexcept
{
  // leading spaces count
  uint32 spaces_count = 0;
  while(spaces_count < str.size())
  {
    if(str[spaces_count] == ' ')
    {
      spaces_count++;
      continue;
    }
    break;
  }

  uint8 tab_width =
    ConfigManager::get_instance()->get_config_struct().tab_width;
  uint32 tabs_count = spaces_count / tab_width;

  str.erase(0, tabs_count * tab_width);

  std::string tab_string(tabs_count, '\t');
  str.insert(0, tab_string);
}

void Buffer::_wrap_selection_with_character(
  const char& wrap_begin_character, const char& wrap_end_character) noexcept
{
  auto sel = this->selection().value();

  _lines[sel.first.first].insert(sel.first.second + 1,
                                 std::string(1, wrap_begin_character));

  // inline selection
  if(_selection.first.first == _selection.second.first)
  {
    _lines[sel.second.first].insert(sel.second.second + 2,
                                    std::string(1, wrap_end_character));
    _selection.first.second += 1;
    _selection.second.second += 1;
    _cursor_col += 1;
    return;
  }

  // multiline selection
  _lines[sel.second.first].insert(sel.second.second + 1,
                                  std::string(1, wrap_end_character));
  if(_selection.first < _selection.second)
  {
    _selection.first.second += 1;
  }
  else
  {
    _selection.second.second += 1;
    _cursor_col += 1;
  }
}

bool Buffer::_cursor_at_bracket() const noexcept
{
  // cursor at start of line
  if(_cursor_col == -1)
  {
    return false;
  }

  const char character = _lines[_cursor_row][_cursor_col];
  if(character == '(' || character == '[' || character == '{')
  {
    return true;
  }

  return false;
}

bool Buffer::_cursor_between_brackets() const noexcept
{
  // cursor is at start or end of line
  if(_cursor_col == -1 || _cursor_col == _lines[_cursor_row].size() - 1)
  {
    return false;
  }

  if((_lines[_cursor_row][_cursor_col] == '(' &&
      _lines[_cursor_row][_cursor_col + 1] == ')') ||
     (_lines[_cursor_row][_cursor_col] == '[' &&
      _lines[_cursor_row][_cursor_col + 1] == ']') ||
     (_lines[_cursor_row][_cursor_col] == '{' &&
      _lines[_cursor_row][_cursor_col + 1] == '}'))
  {
    return true;
  }

  return false;
}
