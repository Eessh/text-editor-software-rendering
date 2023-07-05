#pragma once

#include <string>
#include <vector>
#include "types.hpp"

class Buffer
{
public:
  /// @brief Default constructor.
  Buffer() noexcept;

  /// @brief Creates buffer with the given string.
  /// @param init_string const reference to the string
  ///        with which the buffer should be initialized.
  Buffer(const std::string& init_string) noexcept;

  /// @brief Creates buffer with given lines.
  /// @param lines const reference to vector of strings.
  Buffer(const std::vector<std::string>& lines) noexcept;

  /// @brief Destructor
  ~Buffer() = default;

  [[nodiscard]] bool load_from_file(const std::string& filepath) noexcept;

  [[nodiscard]] uint32 length() const noexcept;

  [[nodiscard]] int32 line_length(const uint32& line_index) const noexcept;

  [[nodiscard]] const std::vector<std::string>& lines() const noexcept;

  [[nodiscard]] const std::string&
  line(const uint32& line_index) const noexcept;

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