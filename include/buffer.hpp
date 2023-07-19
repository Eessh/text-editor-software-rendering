#pragma once

#include <optional>
#include <string>
#include <vector>
#include "types.hpp"

typedef enum class BufferCursorCommand
{
  MOVE_LEFT,
  MOVE_RIGHT,
  MOVE_UP,
  MOVE_DOWN,
} BufferCursorCommand;

typedef enum class BufferSelectionCommand
{
  MOVE_LEFT,
  MOVE_RIGHT,
  MOVE_UP,
  MOVE_DOWN,
  SELECT_LINE
} BufferSelectionCommand;

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

  /// @brief Loads file contents into buffer.
  /// @param filepath path to file.
  /// @return Returns false if unable to load file.
  [[nodiscard]] bool load_from_file(const std::string& filepath) noexcept;

  /// @brief Length of buffer (or) number of lines in buffer.
  /// @return Returns number of lines in unsigned int32 type.
  [[nodiscard]] uint32 length() const noexcept;

  /// @brief Length of line in buffer.
  /// @param line_index index of line.
  /// @return Returns std::nullopt if line_index is out of bounds.
  [[nodiscard]] std::optional<uint32>
  line_length(const uint32& line_index) const noexcept;

  /// @brief Gives buffer in lines format, would be easy for the frontend.
  /// @return Returns const reference to vector of strings.
  [[nodiscard]] const std::vector<std::string>& lines() const noexcept;

  /// @brief Gives content of line in buffer.
  /// @param line_index index of line in buffer (0 based index).
  ///        Check line_index before query.
  /// @return Returns "" if line_index is out of bounds.
  // [[nodiscard]] const std::string&
  // line(const uint32& line_index) const noexcept;
  [[nodiscard]] std::optional<const std::reference_wrapper<std::string>>
  line(const uint32& line_index) const noexcept;

  /// @brief Cursor coordinates in buffer.
  /// @return Returns pair of cursor row (uint32), cursor column (int32).
  [[nodiscard]] std::pair<uint32, int32> cursor_coords() const noexcept;

  /// @brief Gets cursor row.
  /// @return Returns const reference to uint32 cursor row.
  [[nodiscard]] const uint32& cursor_row() const noexcept;

  /// @brief Getter & Setter for cursor row.
  /// @return Returns mutable reference to uint32 cursor row;
  [[nodiscard]] uint32& cursor_row() noexcept;

  /// @brief Gets cursor column.
  /// @return Returns const reference to int32 cursor column.
  [[nodiscard]] const int32& cursor_column() const noexcept;

  /// @brief Getter & Setter for cursor column.
  /// @return Returns mutable reference to int32 cursor column.
  [[nodiscard]] int32& cursor_column() noexcept;

  /// @brief Tells if buffer has selection.
  /// @return Returns false if buffer has no selection.
  [[nodiscard]] bool has_selection() const noexcept;

  /// @brief Gives the selection region in buffer.
  ///        Check if buffer has selection before query.
  /// @return Returns const reference to pair of selection start, end.
  [[nodiscard]] std::pair<std::pair<uint32, int32>, std::pair<uint32, int32>>
  selection() const noexcept;

  /// @brief Executes cursor commands.
  /// @param command the cursor command.
  void execute_cursor_command(const BufferCursorCommand& command) noexcept;

  /// @brief Executes selection commands.
  /// @param command the selection command.
  void
  execute_selection_command(const BufferSelectionCommand& command) noexcept;

  /// @brief Processes BACKSPACE on internal text buffer.
  /// @return Returns true if there is a change in text buffer.
  bool process_backspace() noexcept;

  /// @brief Processes ENTER on internal text buffer.
  void process_enter() noexcept;

  /// @brief Inserts string (without a newline character '\n').
  /// @param str string to insert.
  void insert_string(const std::string& str) noexcept;

private:
  /// @brief Cursor row.
  uint32 _cursor_row;

  /// @brief Cursor column.
  int32 _cursor_col;

  /// @brief Flag for buffer has selection or not.
  bool _has_selection;

  /// @brief Selection region.
  std::pair<std::pair<uint32, int32>, std::pair<uint32, int32>> _selection;

  /// @brief Lines of buffer.
  std::vector<std::string> _lines;

  /// @brief Base function for moving cursor to left.
  ///        Public functions of Buffer do some additional operations
  ///        on top of this function.
  /// @return Returns false if cursor hasn't moved.
  bool _base_move_cursor_left() noexcept;

  /// @brief Base function for moving cursor to right.
  ///        Public functions of Buffer do some additional operations
  ///        on top of this function.
  /// @return Returns false if cursor hasn't moved.
  bool _base_move_cursor_right() noexcept;

  /// @brief Base function for moving cursor up.
  ///        Public functions of Buffer do some additional operations
  ///        on top of this function.
  /// @return Returns false if cursor hasn't moved.
  bool _base_move_cursor_up() noexcept;

  /// @brief Base function for moving cursor down.
  ///        Public functions of Buffer do some additional operations
  ///        on top of this function.
  /// @return Returns false if cursor hasn't moved.
  bool _base_move_cursor_down() noexcept;

  /// @brief Deletes selection from text buffer.
  void _delete_selection() noexcept;
};