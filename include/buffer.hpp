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

  /// @brief Move cursor to previous word starting.
  MOVE_TO_PREVIOUS_WORD_START,

  /// @brief Move cursor to next word ending.
  MOVE_TO_NEXT_WORD_END
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

  /// Extends selection to starting of next word.
  /// If no selection exists, it creates a selection.
  EXTEND_TO_PREVIOUS_WORD_START,

  /// Extends selection to ending of next word.
  /// If no selection exists, it creates a selection.
  EXTENT_TO_NEXT_WORD_END,

  /// @brief Selects word under cursor.
  SELECT_WORD,

  /// @brief Selects whole line under cursor.
  SELECT_LINE
} BufferSelectionCommand;

// /// @brief Buffer view update command type, tells which
// ///        type of command it is: render single line (or)
// ///        render two lines (or) render a range of lines.
// typedef enum class BufferViewUpdateCommandType
// {
//   /// @brief Render a single line - `start_row`.
//   RENDER_LINE,

//   /// Renders a slice of line - `start_row`, `slice_start`, `slice_end`.
//   RENDER_LINE_SLICE,

//   /// Renders line selection - `start_row`.
//   RENDER_LINE_SELECTION,

//   /// Renders line slice selection - `start_row`, `slice_start`, `slice_end`.
//   RENDER_LINE_SLICE_SELECTION,

//   /// @brief Render range of lines, where first line is - start_row,
//   ///        last line is - end_row.
//   RENDER_LINE_RANGE
// } BufferViewUpdateCommandType;

// /// @brief Buffer view update command, tells UI which
// ///        lines are updated, hence should be redrawn.
// struct BufferViewUpdateCommand
// {
//   /// @brief Command type.
//   BufferViewUpdateCommandType type;

//   /// @brief Starting row (for RENDER_LINE_RANGE command type).
//   uint32 start_row;

//   /// @brief Ending row (for RENDER_LINE_RANGE command type).
//   uint32 end_row;

//   /// Starting column of line slice.
//   int32 slice_start;

//   /// Ending column of line slice.
//   int32 slice_end;

//   /// @brief Default constructor.
//   BufferViewUpdateCommand()
//     : type(BufferViewUpdateCommandType::RENDER_LINE)
//     , start_row(0)
//     , end_row(0)
//     , slice_start(-1)
//     , slice_end(-1)
//   {}
// };

class Buffer
{
public:
  /// @brief Default constructor.
  /// @throws No exceptions.
  Buffer() noexcept;

  /// @brief Creates buffer with the given string.
  /// @param init_string const reference to the string
  ///        with which the buffer should be initialized.
  /// @throws No exceptions.
  explicit Buffer(const std::string& init_string) noexcept;

  /// @brief Creates buffer with given lines.
  /// @param lines const reference to vector of strings.
  /// @throws No exceptions.
  explicit Buffer(const std::vector<std::string>& lines) noexcept;

  /// @brief Destructor
  ~Buffer() = default;

  /// @brief Loads file contents into buffer.
  /// @param filepath path to file.
  /// @return Returns false if unable to load file.
  /// @throws No exceptions.
  [[nodiscard]] bool load_from_file(const std::string& filepath) noexcept;

  /// @brief Saves contents to the file.
  /// @returns Returns false if unable to write to file.
  /// @throws No exceptions.
  [[nodiscard]] bool save() noexcept;

  /// @brief Length of buffer (or) number of lines in buffer.
  /// @return Returns number of lines in unsigned int32 type.
  /// @throws No exceptions.
  [[nodiscard]] uint32 length() const noexcept;

  /// @brief Length of line in buffer.
  /// @param line_index index of line.
  /// @return Returns std::nullopt if line_index is out of bounds.
  /// @throws No exceptions.
  [[nodiscard]] std::optional<uint32>
  line_length(const uint32& line_index) const noexcept;

  /// @brief Gives buffer in lines format, would be easy for the frontend.
  /// @return Returns const reference to vector of strings.
  /// @throws No exceptions.
  [[nodiscard]] const std::vector<std::string>& lines() const noexcept;

  /// @brief Gives content of line in buffer.
  /// @param line_index index of line in buffer (0 based index).
  ///        Check line_index before query.
  /// @return Returns std::nullopt if line_index is out of bounds.
  // [[nodiscard]] const std::string&
  // line(const uint32& line_index) const noexcept;
  /// @throws No exceptions.
  [[nodiscard]] std::optional<const std::reference_wrapper<std::string>>
  line(const uint32& line_index) const noexcept;

  /// @brief Gives content of line in buffer, with leading spaces converted
  ///        to indentation tabs.
  /// @param line_index index of line.
  /// @return Returns std::nullopt if line_index is out of bounds.
  /// @throws No exceptions.
  [[nodiscard]] std::optional<std::string>
  line_with_spaces_converted_to_tabs(const uint32& line_index) const noexcept;

  [[nodiscard]] std::optional<uint8>
  line_tab_indent_count_to_show(const uint32& line_index) const noexcept;

  /// @brief Cursor coordinates in buffer.
  /// @return Returns pair of cursor row (uint32), cursor column (int32).
  /// @throws No exceptions.
  [[nodiscard]] std::pair<uint32, int32> cursor_coords() const noexcept;

  /// @brief Gets cursor row.
  /// @return Returns const reference to uint32 cursor row.
  /// @throws No exceptions.
  [[nodiscard]] const uint32& cursor_row() const noexcept;

  /// @brief Getter & Setter for cursor row, this doesn't
  ///        implement effective buffer view updates.
  /// @return Returns mutable reference to uint32 cursor row;
  /// @throws No exceptions.
  [[nodiscard]] uint32& cursor_row() noexcept;

  /// @brief Sets cursor row, this implements effective buffer view updates.
  /// @param row the row to set cursor at.
  /// @throws No exceptions.
  void set_cursor_row(const uint32& row) noexcept;

  /// @brief Gets cursor column.
  /// @return Returns const reference to int32 cursor column.
  /// @throws No exceptions.
  [[nodiscard]] const int32& cursor_column() const noexcept;

  /// @brief Getter & Setter for cursor column, this doesn't
  ///        implement effective buffer view updates.
  /// @return Returns mutable reference to int32 cursor column.
  /// @throws No exceptions.
  [[nodiscard]] int32& cursor_column() noexcept;

  /// @brief Sets cursor column, this implements effective buffer view updates.
  /// @param column the column to set cursor at.
  /// @throws No exceptions.
  void set_cursor_column(const int32& column) noexcept;

  /// @brief Sets cursor's target column.
  ///        Target column is the column cursor tries to reach, when it is
  ///        moving between lines.
  /// @param column_target the target column.
  /// @throws No exceptions.
  void set_cursor_column_target(const int32& column_target) noexcept;

  /// @brief Tells if buffer has selection.
  /// @return Returns false if buffer has no selection.
  /// @throws No exceptions.
  [[nodiscard]] bool has_selection() const noexcept;

  /// @brief Clears selection in buffer.
  /// @throws No exceptions.
  void clear_selection() noexcept;

  /// @brief Gives the selection region in buffer.
  ///        Check if buffer has selection before query.
  /// @return Returns const reference to pair of selection start, end.
  /// @throws No exceptions.
  [[nodiscard]] std::optional<
    std::pair<std::pair<uint32, int32>, std::pair<uint32, int32>>>
  selection() const noexcept;

  /// @brief Gives slice of selection which overlaps with this line.
  /// @param line_index index of line in buffer.
  /// @return Returns std::nullopt if there is no selection or
  ///         line contains no selection.
  /// @throws No exceptions.
  [[nodiscard]] std::optional<std::pair<int32, int32>>
  selection_slice_for_line(const uint32& line_index) const noexcept;

  /// @brief Sets starting point for selection.
  ///        This will be handy when selection is changed by mouse dragging.
  /// @param coordinate the starting point of selection.
  /// @throws No exceptions.
  void set_selection_start_coordinate(
    const std::pair<uint32, int32>& coordinate) noexcept;

  /// @brief Sets ending point for selection.
  ///        This will be handy when selection is changed by mouse dragging.
  /// @param coordinate the ending point for selection.
  /// @throws No exceptions.
  void set_selection_end_coordinate(
    const std::pair<uint32, int32>& coordinate) noexcept;

  /// @brief Executes cursor commands.
  /// @param command the cursor command.
  /// @throws No exceptions.
  void execute_cursor_command(const BufferCursorCommand& command) noexcept;

  /// @brief Executes selection commands.
  /// @param command the selection command.
  /// @throws No exceptions.
  void
  execute_selection_command(const BufferSelectionCommand& command) noexcept;

  /// Extends line selection to the given line.
  /// If no selection is present, selects the given line.
  void extend_line_selection_to_line(const uint32& line_index) noexcept;

  /// @brief Processes BACKSPACE on internal text buffer.
  /// @return Returns true if there is a change in text buffer.
  /// @throws No exceptions.
  bool process_backspace() noexcept;

  /// @brief Processes ENTER on internal text buffer.
  /// @throws No exceptions.
  void process_enter() noexcept;

  /// @brief Inserts string (without a newline character '\n').
  /// @param str string to insert.
  /// @throws No exceptions.
  void insert_string(const std::string& str) noexcept;

  // /// @brief Gets next view update command.
  // /// @return Returns std::nullopt if there are no commands.
  // /// @throws No exceptions.
  // std::optional<BufferViewUpdateCommand>
  // get_next_view_update_command() noexcept;

  /// Gets next incremental render update command.
  std::optional<IncrementalRenderUpdateCommand>
  get_next_incremental_render_update_command() noexcept;

  /// @brief Removes last inserted command, in the view updates queue.
  /// @throws No exceptions.
  void remove_most_recent_view_update_command() noexcept;

  /// @brief Gets next token cache update command.
  /// @return Returns std::nullopt if there are no commands.
  /// @throws No exceptions.
  std::optional<TokenCacheUpdateCommand>
  get_next_token_cache_update_command() noexcept;

private:
  /// @brief Path to the file.
  std::string _file_path;

  /// @brief Cursor row.
  uint32 _cursor_row;

  /// @brief Cursor column.
  int32 _cursor_col;

  /// @brief Target cursor column.
  ///        For natural or expected cursor movement.
  ///        Trust your instincts ^_^
  int32 _cursor_col_target;

  /// @brief Flag for buffer has selection or not.
  bool _has_selection;

  /// @brief Selection region.
  std::pair<std::pair<uint32, int32>, std::pair<uint32, int32>> _selection;

  /// Line selection.
  /// Useful when extending line selection to previous and next lines.
  /// Read this value only if buffer has selection, else
  /// this value maybe of the last selected line.
  uint32 _selected_line;

  /// @brief Lines of buffer.
  std::vector<std::string> _lines;

  // /// @brief View updates queue.
  // std::deque<BufferViewUpdateCommand> _buffer_view_update_commands_queue;

  /// Incremental render update commands.
  std::deque<IncrementalRenderUpdateCommand>
    _buffer_incremental_render_update_commands;

  /// @brief Token cahce updates queue.
  std::deque<TokenCacheUpdateCommand> _token_cache_update_commands_queue;

  /// @brief Base function for moving cursor to left.
  ///        Public functions of Buffer do some additional operations
  ///        on top of this function.
  /// @return Returns false if cursor hasn't moved.
  /// @throws No exceptions.
  bool _base_move_cursor_left() noexcept;

  /// @brief Base function for moving cursor to right.
  ///        Public functions of Buffer do some additional operations
  ///        on top of this function.
  /// @return Returns false if cursor hasn't moved.
  /// @throws No exceptions.
  bool _base_move_cursor_right() noexcept;

  /// @brief Base function for moving cursor up.
  ///        Public functions of Buffer do some additional operations
  ///        on top of this function.
  /// @return Returns false if cursor hasn't moved.
  /// @throws No exceptions.
  bool _base_move_cursor_up() noexcept;

  /// @brief Base function for moving cursor down.
  ///        Public functions of Buffer do some additional operations
  ///        on top of this function.
  /// @return Returns false if cursor hasn't moved.
  /// @throws No exceptions.
  bool _base_move_cursor_down() noexcept;

  /// @brief Base function for moving cursor to previous word starting.
  ///        Public functions of Buffer do some additional operations
  ///        on top of this function.
  /// @return Returns false if cursor hasn't moved.
  /// @throws No exceptions.
  bool _base_move_cursor_to_previous_word_start() noexcept;

  /// @brief Base function for moving cursor to next word ending.
  ///        Public functions of Buffer do some additional operations
  ///        on top of this function.
  /// @return Returns false if cursor hasn't moved.
  /// @throws No exceptions.
  bool _base_move_cursor_to_next_word_end() noexcept;

  /// @brief Deletes selection from text buffer.
  /// @throws No exceptions.
  void _delete_selection() noexcept;

  /// @brief Gives number of leading spaces for line.
  /// @param line_index index of the line.
  /// @return Returns leading spaces count.
  /// @throws No exceptions.
  [[nodiscard]] uint32
  _line_leading_spaces_count(const uint32& line_index) const noexcept;

  /// @brief Converts leading spaces to indentation tabs.
  /// @param str mutable reference to the string.
  /// @throws No exceptions.
  void
  _convert_leading_spaces_to_indentation_tabs(std::string& str) const noexcept;

  /// @brief Wraps selection with given character.
  ///        Check for selection before using this!
  ///        This doesn't check for selection internally.
  /// @param wrap_begin_character the character to insert at start of selection.
  /// @param wrap_end_character the character to insert at end of selection.
  /// @throws No exceptions.
  void _wrap_selection_with_character(const char& wrap_begin_character,
                                      const char& wrap_end_character) noexcept;

  /// @brief Tells if cursor is at bracket: (|, [|, {|
  /// @return
  /// @throws No exceptions.
  [[nodiscard]] bool _cursor_at_bracket() const noexcept;

  /// @brief Tells if cursor is between brackets: (|), [|], {|}.
  /// @return Returns true if cursor is between brackets.
  /// @throws No exceptions.
  [[nodiscard]] bool _cursor_between_brackets() const noexcept;
};
