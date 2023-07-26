#pragma once

#include "../cpp-tokenizer/cpp_tokenizer.hpp"
#include "types.hpp"

/// @brief forward declaration of Buffer class
///        as including buffer.hpp results in tangled includes
class Buffer;

/// @brief Token cache update command type.
enum class TokenCacheUpdateCommandType
{
  /// @brief Re-tokenize whole line - row.
  RETOKENIZE_LINE,

  /// @brief Insert new line after given row, and tokenize it.
  INSERT_NEW_LINE_CACHE_AND_TOKENIZE,

  /// @brief Delete cache for line - row.
  DELETE_LINE_CACHE,

  /// @brief Delete cache for line from - start_row to end_row.
  DELETE_LINES_CACHE
};

/// @brief Token cache update command.
struct TokenCacheUpdateCommand
{
  /// @brief Type of command.
  TokenCacheUpdateCommandType type;

  /// @brief Row to re-tokenize (or) insert new line after this row
  ///        and tokenize it (or) delete token cache
  ///        (for RETOKENIZE_LINE, INSERT_NEW_LINE_CACHE_AND_TOKENIZE,
  ///        DELETE_LINE_CACHE command types).
  uint32 row;

  /// @brief Starting row to delete lines cache (for DELETE_LINES_CACHE
  ///        command type).
  uint32 start_row;

  /// @brief Ending row to delete lines cache (for DELETE_LINES_CACHE
  ///        command type).
  uint32 end_row;
};

class CppTokenizerCache
{
public:
  CppTokenizerCache() = default;

  /// @brief Builds intial token cache for all lines in buffer.
  /// @param buffer const reference to buffer.
  void build_cache(const Buffer& buffer) noexcept;

  /// @brief Incrementally updates the token cache
  ///        from lines updated in buffer.
  /// @param buffer const reference to buffer.
  void update_cache(Buffer& buffer) noexcept;

  /// @brief Gives tokens line-wise.
  /// @return const reference to lines of tokens.
  const std::vector<std::vector<CppTokenizer::Token>>& tokens() const noexcept;

private:
  /// @brief Token cache.
  std::vector<std::vector<CppTokenizer::Token>> _tokens;

  /// @brief CPP Tokenizer.
  CppTokenizer::Tokenizer _tokenizer;

  /// @brief Re-tokenized lines in update_cache method.
  std::vector<uint32> _re_tokenized_lines;
};