#include "../include/buffer.hpp"
#include <fstream>
#include "../include/macros.hpp"

Buffer::Buffer() noexcept
  : _cursor_row(0)
  , _cursor_col(-1)
  , _has_selection(false)
  , _selection({{0, -1}, {0, -1}})
  , _lines({""})
  , _buffer_view_update_commands_queue(std::deque<BufferViewUpdateCommand>())
{}

Buffer::Buffer(const std::string& init_string) noexcept
  : _cursor_row(0)
  , _cursor_col(-1)
  , _has_selection(false)
  , _selection({{0, -1}, {0, -1}})
  , _lines({init_string})
  , _buffer_view_update_commands_queue(std::deque<BufferViewUpdateCommand>())
{}

Buffer::Buffer(const std::vector<std::string>& lines) noexcept
  : _cursor_row(0)
  , _cursor_col(-1)
  , _has_selection(false)
  , _selection({{0, -1}, {0, -1}})
  , _lines(lines)
  , _buffer_view_update_commands_queue(std::deque<BufferViewUpdateCommand>())
{}

bool Buffer::load_from_file(const std::string& filepath) noexcept
{
  std::ifstream file(filepath);
  if(file.is_open()) [[likely]]
  {
    _lines.clear();
    while(file.good())
    {
      _lines.emplace_back("");
      std::getline(file, _lines.back());
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
  BufferViewUpdateCommand cmd;
  cmd.type = BufferViewUpdateCommandType::RENDER_LINES;
  // cmd.rows = {_cursor_row, row};
  cmd.old_active_line = _cursor_row;
  cmd.new_active_line = row;
  _buffer_view_update_commands_queue.push_back(cmd);
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
  BufferViewUpdateCommand cmd;
  cmd.type = BufferViewUpdateCommandType::RENDER_LINE;
  cmd.row = _cursor_row;
  _buffer_view_update_commands_queue.push_back(cmd);
  _cursor_col = column;
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

  auto selection = this->selection();
  BufferViewUpdateCommand cmd;
  cmd.type = BufferViewUpdateCommandType::RENDER_LINE_RANGE;
  cmd.start_row = selection.first.first;
  cmd.end_row = selection.second.first;
  _buffer_view_update_commands_queue.push_back(cmd);
  _has_selection = false;
}

std::pair<std::pair<uint32, int32>, std::pair<uint32, int32>>
Buffer::selection() const noexcept
{
  if(!_has_selection) [[unlikely]]
  {
    ERROR_BOII("Called GET selection, when buffer has no selection!");
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

  auto selection = this->selection();
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

void Buffer::execute_cursor_command(const BufferCursorCommand& command) noexcept
{
  switch(command)
  {
  case BufferCursorCommand::MOVE_LEFT: {
    if(_has_selection)
    {
      auto selection = this->selection();
      _cursor_row = selection.first.first;
      _cursor_col = selection.first.second;
      _has_selection = false;
      BufferViewUpdateCommand cmd;
      cmd.type = BufferViewUpdateCommandType::RENDER_LINE_RANGE;
      cmd.start_row = selection.first.first;
      cmd.end_row = selection.second.first;
      _buffer_view_update_commands_queue.push_back(cmd);
      return;
    }

    this->_base_move_cursor_left();

    break;
  }
  case BufferCursorCommand::MOVE_RIGHT: {
    if(_has_selection)
    {
      auto selection = this->selection();
      _cursor_row = selection.second.first;
      _cursor_col = selection.second.second;
      _has_selection = false;
      BufferViewUpdateCommand cmd;
      cmd.type = BufferViewUpdateCommandType::RENDER_LINE_RANGE;
      cmd.start_row = selection.first.first;
      cmd.end_row = selection.second.first;
      _buffer_view_update_commands_queue.push_back(cmd);
      return;
    }

    this->_base_move_cursor_right();

    break;
  }
  case BufferCursorCommand::MOVE_UP: {
    if(_has_selection)
    {
      auto selection = this->selection();
      _cursor_row = selection.first.first;
      _cursor_col = selection.first.second;
      _has_selection = false;
      BufferViewUpdateCommand cmd;
      cmd.type = BufferViewUpdateCommandType::RENDER_LINE_RANGE;
      cmd.end_row = selection.second.first;
      if(_cursor_row == 0)
      {
        cmd.start_row = selection.first.first;
        _buffer_view_update_commands_queue.push_back(cmd);
        return;
      }
      --_cursor_row;
      cmd.start_row = _cursor_row;
      if(static_cast<int32>(_lines[_cursor_row].size() - 1) < _cursor_col)
      {
        _cursor_col = _lines[_cursor_row].size() - 1;
      }
      _buffer_view_update_commands_queue.push_back(cmd);
      return;
    }

    this->_base_move_cursor_up();

    break;
  }
  case BufferCursorCommand::MOVE_DOWN: {
    if(_has_selection)
    {
      auto selection = this->selection();
      _cursor_row = selection.second.first;
      _cursor_col = selection.second.second;
      _has_selection = false;
      BufferViewUpdateCommand cmd;
      cmd.type = BufferViewUpdateCommandType::RENDER_LINE_RANGE;
      cmd.start_row = selection.first.first;
      if(_cursor_row == _lines.size() - 1)
      {
        cmd.end_row = selection.second.first;
        _buffer_view_update_commands_queue.push_back(cmd);
        return;
      }
      ++_cursor_row;
      cmd.end_row = _cursor_row;
      if(static_cast<int32>(_lines[_cursor_row].size() - 1) < _cursor_col)
      {
        _cursor_col = _lines[_cursor_row].size() - 1;
      }
      _buffer_view_update_commands_queue.push_back(cmd);
      return;
    }

    this->_base_move_cursor_down();

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
    BufferViewUpdateCommand cmd;
    cmd.type = BufferViewUpdateCommandType::RENDER_LINE;
    cmd.row = _cursor_row;
    _buffer_view_update_commands_queue.push_back(cmd);
    break;
  }
  case BufferSelectionCommand::SELECT_LINE: {
    _has_selection = true;
    _selection.first.first = _cursor_row;
    _selection.first.second = -1;
    if(_cursor_row == _lines.size() - 1)
    {
      // ending line
      _selection.second.first = _cursor_row;
      _selection.second.second = _lines[_cursor_row].size() - 1;
      _cursor_col = _lines[_cursor_row].size() - 1;
      BufferViewUpdateCommand cmd;
      cmd.type = BufferViewUpdateCommandType::RENDER_LINE;
      cmd.row = _cursor_row;
      _buffer_view_update_commands_queue.push_back(cmd);
    }
    else
    {
      _selection.second.first = _cursor_row + 1;
      _selection.second.second = -1;
      _cursor_col = -1;
      BufferViewUpdateCommand cmd;
      cmd.type = BufferViewUpdateCommandType::RENDER_LINES;
      cmd.old_active_line = _cursor_row;
      _cursor_row += 1;
      cmd.new_active_line = _cursor_row;
      _buffer_view_update_commands_queue.push_back(cmd);
    }
    break;
  }
  default:
    break;
  }
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
    // remove character before cursor
    _lines[_cursor_row].erase(_cursor_col, 1);
    _cursor_col -= 1;
    BufferViewUpdateCommand cmd;
    cmd.type = BufferViewUpdateCommandType::RENDER_LINE;
    cmd.row = _cursor_row;
    _buffer_view_update_commands_queue.push_back(cmd);
    return true;
  }

  // append the contents of this string to above line
  _cursor_col = _lines[_cursor_row - 1].size() - 1;
  _lines[_cursor_row - 1].append(_lines[_cursor_row]);
  _lines.erase(_lines.begin() + _cursor_row);
  _cursor_row -= 1;
  BufferViewUpdateCommand cmd;
  cmd.type = BufferViewUpdateCommandType::RENDER_LINE_RANGE;
  cmd.start_row = _cursor_row;
  cmd.end_row = _lines.size() - 1;
  _buffer_view_update_commands_queue.push_back(cmd);
  return true;
}

void Buffer::process_enter() noexcept
{
  if(_has_selection)
  {
    this->_delete_selection();
  }

  // insert new line after this line
  // and append contents of this line after the cursor to new line
  _lines.insert(_lines.begin() + _cursor_row + 1, "");
  _lines[_cursor_row + 1].append(_lines[_cursor_row].substr(_cursor_col + 1));
  _lines[_cursor_row].erase(_cursor_col + 1);
  _cursor_col = -1;
  BufferViewUpdateCommand cmd;
  cmd.type = BufferViewUpdateCommandType::RENDER_LINE_RANGE;
  cmd.start_row = _cursor_row;
  _cursor_row += 1;
  cmd.end_row = _lines.size() - 1;
  _buffer_view_update_commands_queue.push_back(cmd);
}

void Buffer::insert_string(const std::string& str) noexcept
{
  if(_has_selection)
  {
    this->_delete_selection();
  }

  _lines[_cursor_row].insert(_cursor_col + 1, str);
  _cursor_col += str.size();
  BufferViewUpdateCommand cmd;
  cmd.type = BufferViewUpdateCommandType::RENDER_LINE;
  cmd.row = _cursor_row;
  _buffer_view_update_commands_queue.push_back(cmd);
}

std::optional<BufferViewUpdateCommand>
Buffer::get_next_view_update_command() noexcept
{
  if(_buffer_view_update_commands_queue.empty())
  {
    return std::nullopt;
  }

  BufferViewUpdateCommand command = _buffer_view_update_commands_queue.front();
  _buffer_view_update_commands_queue.pop_front();
  return command;
}

void Buffer::remove_most_recent_command() noexcept
{
  if(_buffer_view_update_commands_queue.empty())
  {
    return;
  }

  _buffer_view_update_commands_queue.pop_back();
}

bool Buffer::_base_move_cursor_left() noexcept
{
  if(_cursor_col > -1) [[likely]]
  {
    _cursor_col--;
    BufferViewUpdateCommand cmd;
    cmd.type = BufferViewUpdateCommandType::RENDER_LINE;
    cmd.row = _cursor_row;
    _buffer_view_update_commands_queue.push_back(cmd);
    return true;
  }

  if(_cursor_row == 0)
  {
    return false;
  }

  BufferViewUpdateCommand cmd;
  cmd.type = BufferViewUpdateCommandType::RENDER_LINES;
  cmd.old_active_line = _cursor_row;
  --_cursor_row;
  _cursor_col = _lines[_cursor_row].size() - 1;
  cmd.new_active_line = _cursor_row;
  _buffer_view_update_commands_queue.push_back(cmd);

  return true;
}

bool Buffer::_base_move_cursor_right() noexcept
{
  if(_cursor_col < static_cast<int32>(_lines[_cursor_row].size() - 1))
    [[likely]]
  {
    _cursor_col++;
    BufferViewUpdateCommand cmd;
    cmd.type = BufferViewUpdateCommandType::RENDER_LINE;
    cmd.row = _cursor_row;
    _buffer_view_update_commands_queue.push_back(cmd);
    return true;
  }

  if(_cursor_row == _lines.size() - 1)
  {
    return false;
  }

  BufferViewUpdateCommand cmd;
  cmd.type = BufferViewUpdateCommandType::RENDER_LINES;
  cmd.old_active_line = _cursor_row;
  ++_cursor_row;
  _cursor_col = -1;
  cmd.new_active_line = _cursor_row;
  _buffer_view_update_commands_queue.push_back(cmd);

  return true;
}

bool Buffer::_base_move_cursor_up() noexcept
{
  if(_cursor_row == 0)
  {
    return false;
  }

  BufferViewUpdateCommand cmd;
  cmd.type = BufferViewUpdateCommandType::RENDER_LINES;
  cmd.old_active_line = _cursor_row;
  --_cursor_row;
  if(static_cast<int32>(_lines[_cursor_row].size() - 1) < _cursor_col)
  {
    _cursor_col = _lines[_cursor_row].size() - 1;
  }
  cmd.new_active_line = _cursor_row;
  _buffer_view_update_commands_queue.push_back(cmd);

  return true;
}

bool Buffer::_base_move_cursor_down() noexcept
{
  if(_cursor_row == _lines.size() - 1)
  {
    return false;
  }

  BufferViewUpdateCommand cmd;
  cmd.type = BufferViewUpdateCommandType::RENDER_LINES;
  cmd.old_active_line = _cursor_row;
  ++_cursor_row;
  if(static_cast<int32>(_lines[_cursor_row].size() - 1) < _cursor_col)
  {
    _cursor_col = _lines[_cursor_row].size() - 1;
  }
  cmd.new_active_line = _cursor_row;
  _buffer_view_update_commands_queue.push_back(cmd);

  return true;
}

void Buffer::_delete_selection() noexcept
{
  if(!_has_selection)
  {
    return;
  }

  // delete selection
  auto selection = this->selection();
  if(selection.first.first == selection.second.first)
  {
    // deletion happens in same line
    _lines[selection.first.first].erase(selection.first.second + 1,
                                        selection.second.second -
                                          selection.first.second);
    BufferViewUpdateCommand cmd;
    cmd.type = BufferViewUpdateCommandType::RENDER_LINE;
    cmd.row = _cursor_row;
    _buffer_view_update_commands_queue.push_back(cmd);
  }
  else
  {
    BufferViewUpdateCommand cmd;
    cmd.type = BufferViewUpdateCommandType::RENDER_LINE_RANGE;
    cmd.start_row = selection.first.first;
    cmd.end_row = _lines.size() - 1;
    _buffer_view_update_commands_queue.push_back(cmd);

    _lines[selection.first.first].erase(selection.first.second + 1);
    _lines[selection.second.first].erase(0, selection.second.second + 1);
    _lines[selection.first.first].append(_lines[selection.second.first]);
    // deleting lines btw selection start end, which are fully selected
    _lines.erase(_lines.begin() + selection.first.first + 1,
                 _lines.begin() + selection.second.first + 1);
  }
  _cursor_row = selection.first.first;
  _cursor_col = selection.first.second;
  _has_selection = false;
}