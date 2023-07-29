#pragma once

#include <deque>
#include <optional>
#include <string>
#include <vector>
#include "cpp_tokenizer_cache.hpp"
#include "types.hpp"

/// @brief Cursor commands to process on buffer.
typedef enum class BufferCursorCommand
{
  /// @brief Move cursor left. If already at start of line,
  ///        it moves to the end of previous line (if exists).
  MOVE_LEFT,

  /// @brief Move cursor right. If already at end of line,
  ///        it moves to the start of next line (if exists).
  MOVE_RIGHT,

  /// @brief Move cursor up.
  MOVE_UP,

  /// @brief Move cursor down.
  MOVE_DOWN,
} BufferCursorCommand;

/// @brief Selection commands to process on buffer.
typedef enum class BufferSelectionCommand
{
  /// @brief Extends selection leftward. If no selection exists,
  ///        it creates a selection.
  MOVE_LEFT,

  /// @brief Extends selection rightward. If no selection exists,
  ///        it creates a selection.
  MOVE_RIGHT,

  /// @brief Extends selection upward. If no selection exists,
  ///        it creates a selection.
  MOVE_UP,

  /// @brief Extends selection downward. If no selection exists,
  ///        it creates a selection.
  MOVE_DOWN,

  /// @brief Selects word under cursor.
  SELECT_WORD,

  /// @brief Selects whole line under cursor.
  SELECT_LINE
} BufferSelectionCommand;

/// @brief Buffer view update command type, tells which
///        type of command it is: render single line (or)
///        render two lines (or) render a range of lines.
typedef enum class BufferViewUpdateCommandType
{
  /// @brief Render a single line - row.
  RENDER_LINE,

  /// @brief Render two lines, where first line is - old_active_line,
  ///        second line is - new_active_line.
  RENDER_LINES,

  /// @brief Render range of lines, where first line is - start_row,
  ///        last line is - end_row.
  RENDER_LINE_RANGE
} BufferViewUpdateCommandType;

/// @brief Buffer view update command, tells UI which
///        lines are updated, hence should be redrawn.
struct BufferViewUpdateCommand
{
  /// @brief Command type.
  BufferViewUpdateCommandType type;

  /// @brief Row to render (for RENDER_LINE command type).
  uint32 row;

  /// @brief Old active line (for RENDER_LINES command type).
  uint32 old_active_line;

  /// @brief New active line (for RENDER_LINES command type).
  uint32 new_active_line;

  /// @brief Starting row (for RENDER_LINE_RANGE command type).
  uint32 start_row;

  /// @brief Ending row (for RENDER_LINE_RANGE command type).
  uint32 end_row;

  /// @brief Default constructor.
  BufferViewUpdateCommand()
    : type(BufferViewUpdateCommandType::RENDER_LINE)
    , row(0)
    , old_active_line(0)
    , new_active_line(0)
    , start_row(0)
    , end_row(0)
  {}
};

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
  /// @return Returns std::nullopt if line_index is out of bounds.
  // [[nodiscard]] const std::string&
  // line(const uint32& line_index) const noexcept;
  [[nodiscard]] std::optional<const std::reference_wrapper<std::string>>
  line(const uint32& line_index) const noexcept;

  /// @brief Gives content of line in buffer, with leading spaces converted
  ///        to indentation tabs.
  /// @param line_index index of line.
  /// @return Returns std::nullopt if line_index is out of bounds.
  [[nodiscard]] std::optional<std::string>
  line_with_spaces_converted_to_tabs(const uint32& line_index) const noexcept;

  /// @brief Cursor coordinates in buffer.
  /// @return Returns pair of cursor row (uint32), cursor column (int32).
  [[nodiscard]] std::pair<uint32, int32> cursor_coords() const noexcept;

  /// @brief Gets cursor row.
  /// @return Returns const reference to uint32 cursor row.
  [[nodiscard]] const uint32& cursor_row() const noexcept;

  /// @brief Getter & Setter for cursor row, this doesn't
  ///        implement effective buffer view updates.
  /// @return Returns mutable reference to uint32 cursor row;
  [[nodiscard]] uint32& cursor_row() noexcept;

  /// @brief Sets cursor row, this implements effective buffer view updates.
  /// @param row the row to set cursor at.
  void set_cursor_row(const uint32& row) noexcept;

  /// @brief Gets cursor column.
  /// @return Returns const reference to int32 cursor column.
  [[nodiscard]] const int32& cursor_column() const noexcept;

  /// @brief Getter & Setter for cursor column, this doesn't
  ///        implement effective buffer view updates.
  /// @return Returns mutable reference to int32 cursor column.
  [[nodiscard]] int32& cursor_column() noexcept;

  /// @brief Sets cursor column, this implements effective buffer view updates.
  /// @param column the column to set cursor at.
  void set_cursor_column(const int32& column) noexcept;

  /// @brief Tells if buffer has selection.
  /// @return Returns false if buffer has no selection.
  [[nodiscard]] bool has_selection() const noexcept;

  /// @brief Clears selection in buffer.
  void clear_selection() noexcept;

  /// @brief Gives the selection region in buffer.
  ///        Check if buffer has selection before query.
  /// @return Returns const reference to pair of selection start, end.
  [[nodiscard]] std::optional<
    std::pair<std::pair<uint32, int32>, std::pair<uint32, int32>>>
  selection() const noexcept;

  /// @brief Gives slice of selection which overlaps with this line.
  /// @param line_index index of line in buffer.
  /// @return Returns std::nullopt if there is no selection or
  ///         line contains no selection.
  [[nodiscard]] std::optional<std::pair<int32, int32>>
  selection_slice_for_line(const uint32& line_index) const noexcept;

  /// @brief Sets starting point for selection.
  ///        This will be handy when selection is changed by mouse dragging.
  /// @param coordinate the starting point of selection.
  void set_selection_start_coordinate(
    const std::pair<uint32, int32>& coordinate) noexcept;

  /// @brief Sets ending point for selection.
  ///        This will be handy when selection is changed by mouse dragging.
  /// @param coordinate the ending point for selection.
  void set_selection_end_coordinate(
    const std::pair<uint32, int32>& coordinate) noexcept;

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

  /// @brief Gets next view update command.
  /// @return Returns std::nullopt if there are no commands.
  std::optional<BufferViewUpdateCommand>
  get_next_view_update_command() noexcept;

  /// @brief Removes last inserted command, in the view updates queue.
  void remove_most_recent_view_update_command() noexcept;

  /// @brief Gets next token cache update command.
  /// @return Returns std::nullopt if there are no commands.
  std::optional<TokenCacheUpdateCommand>
  get_next_token_cache_update_command() noexcept;

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

  /// @brief View updates queue.
  std::deque<BufferViewUpdateCommand> _buffer_view_update_commands_queue;

  /// @brief Token cahce updates queue.
  std::deque<TokenCacheUpdateCommand> _token_cache_update_commands_queue;

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

  /// @brief Gives number of leading spaces for line.
  /// @param line_index index of the line.
  /// @return Returns leading spaces count.
  uint32 _line_leading_spaces_count(const uint32& line_index) const noexcept;

  /// @brief Converts leading spaces to indentation tabs.
  /// @param str mutable reference to the string.
  void
  _convert_leading_spaces_to_indentation_tabs(std::string& str) const noexcept;

  /// @brief Wraps selection with given character.
  ///        Check for selection before using this!
  ///        This doesn't check for selection internally.
  /// @param character the character to wrap selection with.
  void _wrap_selection_with_character(const char& wrap_begin_character,
                                      const char& wrap_end_character) noexcept;

  /// @brief Tells if cursor is between brackets: (|), [|], {|}.
  /// @return Returns true if cursor is between brackets.
  bool _cursor_between_brackets() const noexcept;
};