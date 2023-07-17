#include "../include/buffer.hpp"
#include <fstream>
#include "../include/macros.hpp"

Buffer::Buffer() noexcept
  : _cursor_row(0)
  , _cursor_col(-1)
  , _has_selection(false)
  , _selection({{0, -1}, {0, -1}})
  , _lines({""})
{}

Buffer::Buffer(const std::string& init_string) noexcept
  : _cursor_row(0)
  , _cursor_col(-1)
  , _has_selection(false)
  , _selection({{0, -1}, {0, -1}})
  , _lines({init_string})
{}

Buffer::Buffer(const std::vector<std::string>& lines) noexcept
  : _cursor_row(0)
  , _cursor_col(-1)
  , _has_selection(false)
  , _selection({{0, -1}, {0, -1}})
  , _lines(lines)
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

const std::string& Buffer::line(const uint32& line_index) const noexcept
{
  if(line_index >= _lines.size()) [[unlikely]]
  {
    ERROR_BOII("Accessing line with line_index out of bounds!");
    return "";
  }
  else [[likely]]
  {
    return _lines[line_index];
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

const int32& Buffer::cursor_column() const noexcept
{
  return _cursor_col;
}

int32& Buffer::cursor_column() noexcept
{
  return _cursor_col;
}

bool Buffer::has_selection() const noexcept
{
  return _has_selection;
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
      if(_cursor_row == 0)
      {
        return;
      }
      --_cursor_row;
      if(static_cast<int32>(_lines[_cursor_row].size() - 1) < _cursor_col)
      {
        _cursor_col = _lines[_cursor_row].size() - 1;
      }
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
      if(_cursor_row == _lines.size() - 1)
      {
        return;
      }
      ++_cursor_row;
      if(static_cast<int32>(_lines[_cursor_row].size() - 1) < _cursor_col)
      {
        _cursor_col = _lines[_cursor_row].size() - 1;
      }
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

    this->_base_move_cursor_down();
    _selection.second.first = _cursor_row;
    _selection.second.second = _cursor_col;
    break;
  }
  default:
    break;
  }
}

bool Buffer::_base_move_cursor_left() noexcept
{
  if(_cursor_col > -1)
  {
    _cursor_col--;
    return true;
  }

  if(_cursor_row == 0)
  {
    return false;
  }

  --_cursor_row;
  _cursor_col = _lines[_cursor_row].size() - 1;

  return true;
}

bool Buffer::_base_move_cursor_right() noexcept
{
  if(_cursor_col < static_cast<int32>(_lines[_cursor_row].size() - 1))
  {
    _cursor_col++;
    return true;
  }

  if(_cursor_row == _lines.size() - 1)
  {
    return false;
  }

  ++_cursor_row;
  _cursor_col = -1;

  return true;
}

bool Buffer::_base_move_cursor_up() noexcept
{
  if(_cursor_row == 0)
  {
    return false;
  }

  --_cursor_row;
  if(static_cast<int32>(_lines[_cursor_row].size() - 1) < _cursor_col)
  {
    _cursor_col = _lines[_cursor_row].size() - 1;
  }

  return true;
}

bool Buffer::_base_move_cursor_down() noexcept
{
  if(_cursor_row == _lines.size() - 1)
  {
    return false;
  }

  ++_cursor_row;
  if(static_cast<int32>(_lines[_cursor_row].size() - 1) < _cursor_col)
  {
    _cursor_col = _lines[_cursor_row].size() - 1;
  }

  return true;
}