#pragma once

#include <string>
#include <vector>
#include "types.hpp"

class Buffer
{
public:
  /// @brief Default constructor.
  Buffer();

  /// @brief Creates buffer with the given string.
  /// @param init_string const reference to the string
  ///        with which the buffer should be initialized.
  Buffer(const std::string& init_string);

  /// @brief Creates buffer with given lines.
  /// @param lines const reference to vector of strings.
  Buffer(const std::vector<std::string>& lines);

  /// @brief Destructor
  ~Buffer() = default;

  [[nodiscard]] bool load_from_file(const std::string& filepath) noexcept;

  [[nodiscard]] const std::vector<std::string>& lines() const noexcept;

  [[nodiscard]] std::pair<uint32, int32> cursor_coords() const noexcept;

  [[nodiscard]] bool has_selection() const noexcept;

  [[nodiscard]] const std::pair<std::pair<uint32, int32>,
                                std::pair<uint32, int32>>&
  selection() const noexcept;

private:
  uint32 _cursor_row;
  int32 _cursor_col;
  bool _has_selection;
  std::pair<std::pair<uint32, int32>, std::pair<uint32, int32>> _selection;
  std::vector<std::string> _lines;
};