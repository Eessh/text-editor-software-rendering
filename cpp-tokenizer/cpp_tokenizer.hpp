#pragma once

#include <optional>
#include <string>
#include <vector>

/// @brief CppTokenizer namespace.
namespace CppTokenizer
{

/// @brief TokenType - enum of all token types.
typedef enum class TokenType
{
  /// @brief Whitespace - ' ' character.
  WHITESPACE,
  /// @brief Newline - '\n' character.
  NEWLINE,
  /// @brief Tab - '\t' character.
  TAB,

  /// @brief Semicolon - ';' character.
  SEMICOLON,
  /// @brief Coma - ',' character.
  COMMA,

  /// @brief Escape backslash - '\' character.
  ESCAPE_BACKSLASH,

  /// @brief Open bracket - '(' character.
  BRACKET_OPEN,
  /// @brief Closed bracket - ')' character.
  BRACKET_CLOSE,

  /// @brief Open square bracket - '[' character.
  SQUARE_BRACKET_OPEN,
  /// @brief Closed square bracket - ']' character.
  SQUARE_BRACKET_CLOSE,

  /// @brief Open curly brace - '{' character.
  CURLY_BRACE_OPEN,
  /// @brief Closed curly brace - '}' character.
  CURLY_BRACE_CLOSE,

  /// @brief Charcter token type.
  CHARACTER,
  /// @brief Strng token type.
  STRING,
  /// @brief Comment token type.
  COMMENT,
  /// @brief Multiline token type.
  MULTILINE_COMMENT,
  /// @brief Incomplete multiline token type.
  ///        Incomplete means this token is not closed by "*/"
  MULTILINE_COMMENT_INCOMPLETE,
  /// @brief Operator token type.
  OPERATOR,
  /// @brief Keyword token type.
  KEYWORD,
  /// @brief Preprocessor directive token type.
  PREPROCESSOR_DIRECTIVE,
  /// @brief Identifier token type.
  IDENTIFIER,
  /// @brief Number token type.
  NUMBER,
  /// @brief Function token type.
  FUNCTION,
  /// @brief Header token type.
  HEADER,

  /// @brief Unknown token type.
  UNKNOWN
} TokenType;

/// @brief Token - consists of type, start & end offsets, token string value.
class Token
{
public:
  /// @brief Default constructor.
  Token() noexcept;

  /// @brief Constructor with token type argument.
  /// @param token_type the type of token to construct.
  Token(const TokenType& token_type) noexcept;

  /// @brief Checks if given token is not equal to this token.
  /// @param other the token to compare with this token.
  /// @return Returns true if the tokens are not equal.
  bool operator!=(const Token& other) const noexcept;

  /// @brief Type of token.
  TokenType type;

  /// @brief Starting offset or position of token in string.
  uint32_t start_offset;

  /// @brief Ending offset or position of token in string.
  uint32_t end_offset;

  /// @brief Token string.
  std::string value;
};

class Tokenizer
{
public:
  /// @brief Default constructor.
  Tokenizer() noexcept;

  /// @brief Tokenizes the given string into tokens.
  /// @param str const reference to the string to tokenize.
  /// @return Const reference to vector of Tokens.
  [[nodiscard]] const std::vector<Token>&
  tokenize(const std::string& str) noexcept;

  [[nodiscard]] const std::vector<Token>&
  tokenize_from_imcomplete_token(const std::string& str,
                                 const Token& incomplete_token) noexcept;

  /// @brief Clears tokens stored in previous tokenization.
  void clear_tokens() noexcept;

private:
  Token _current_token;
  std::vector<Token> _tokens;
  bool _inside_string, _inside_char, _inside_comment, _inside_multiline_comment;
  uint32_t _position;

  [[nodiscard]] bool inside_include_declaration() const noexcept;
};

/// @brief Logs tokens to STDOUT with pretty format.
/// @param tokens const reference to vector of tokens.
void log_tokens(const std::vector<Token>& tokens) noexcept;

}; // namespace CppTokenizer