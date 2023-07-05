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

const std::vector<std::string>& Buffer::lines() const noexcept
{
  return _lines;
}

std::pair<uint32, int32> Buffer::cursor_coords() const noexcept
{
  return std::make_pair(_cursor_row, _cursor_col);
}

bool Buffer::has_selection() const noexcept
{
  return _has_selection;
}

const std::pair<std::pair<uint32, int32>, std::pair<uint32, int32>>&
Buffer::selection() const noexcept
{
  if(!_has_selection) [[unlikely]]
  {
    ERROR_BOII("Called GET selection, when buffer has no selection!");
  }
  return _selection;
}