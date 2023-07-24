#pragma once

#include "../cpp-tokenizer/cpp_tokenizer.hpp"
#include "buffer.hpp"
#include "types.hpp"

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
  void update_cache(const Buffer& buffer) noexcept;

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