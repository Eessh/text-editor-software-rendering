#include "cpp_tokenizer.hpp"

// static std::vector<std::string> seperators = {
//   " ", "\n", ".", "!", "\t", ";", ":", "\\", "/", "+", "-",  "*",  "&",
//   "%", "<",  ">", "=", "(",  ")", "{", "}",  "[", "]", "\"", "\'", ","};

// static std::vector<char> char_seps = {
//   ' ', '\n', '.', '!', '\t', ';', ':', '\\', '/', '+', '-', '*', '&',
//   '%', '<', '>', '=', '(', ')', '{', '}', '[', ']', '"', '\'', ','
// };

static std::string seperators = " \n\r.!\t;:\\/+-*&%<>=(){}[]\"',|~^";

static std::vector<std::string> operators = {
  "::", "->", "<=", ">=", "+=", "-=", "/=", "*=", "^=", "&&", "==",
  "&=", "||", "%=", ">>", "<<", "~",  "+",  "-",  "*",  "/",  "=",
  "<",  ">",  "!",  "?",  ":",  "^",  "&",  "|",  "%",  "."};

static std::vector<std::string> keywords = {"alignas",
                                            "alignof",
                                            "and_eq",
                                            "and",
                                            "asm",
                                            "atomic_cancel",
                                            "atomic_commit",
                                            "atomic_noexcept",
                                            "auto",
                                            "bitand",
                                            "bitor",
                                            "bool",
                                            "break",
                                            "case",
                                            "catch",
                                            "char8_t",
                                            "char16_t",
                                            "char32_t",
                                            "char",
                                            "class",
                                            "compl",
                                            "concept",
                                            "consteval",
                                            "constexpr",
                                            "constinit",
                                            "const_cast",
                                            "const",
                                            "continue",
                                            "co_await",
                                            "co_return",
                                            "co_yeild",
                                            "decltype",
                                            "default",
                                            "delete",
                                            "double",
                                            "do",
                                            "dynamic_cast",
                                            "else",
                                            "enum",
                                            "explicit",
                                            "export",
                                            "extern",
                                            "false",
                                            "float",
                                            "for",
                                            "friend",
                                            "goto",
                                            "if",
                                            "inline",
                                            "int",
                                            "long",
                                            "mutable",
                                            "namespace",
                                            "new",
                                            "noexcept",
                                            "not_eq",
                                            "not",
                                            "nullptr",
                                            "operator",
                                            "or_eq",
                                            "or",
                                            "private",
                                            "protected",
                                            "public",
                                            "reflexpr",
                                            "register",
                                            "reinterpret_cast",
                                            "requires",
                                            "return",
                                            "short",
                                            "signed",
                                            "sizeof",
                                            "static_assert",
                                            "static_cast",
                                            "static",
                                            "struct",
                                            "switch",
                                            "synchronized",
                                            "template",
                                            "this",
                                            "thread_local",
                                            "throw",
                                            "true",
                                            "try",
                                            "typedef",
                                            "typeid",
                                            "typename",
                                            "union",
                                            "unsigned",
                                            "using",
                                            "virtual",
                                            "void",
                                            "volatile",
                                            "wchar_t",
                                            "while",
                                            "xor_eq",
                                            "xor"};

static std::vector<std::string> preprocessor_directives = {"#ifndef",
                                                           "#ifdef",
                                                           "#if",
                                                           "#else",
                                                           "#elif",
                                                           "#elifdef",
                                                           "#elifndef",
                                                           "#endif",
                                                           "#define",
                                                           "#undef",
                                                           "#include",
                                                           "#error",
                                                           "#warning",
                                                           "#pragma",
                                                           "#line"};

std::string token_type_to_string(const CppTokenizer::TokenType& type)
{
  switch(type)
  {
  case CppTokenizer::TokenType::WHITESPACE:
    return "WHITESPACE";
  case CppTokenizer::TokenType::NEWLINE:
    return "NEWLINe";
  case CppTokenizer::TokenType::TAB:
    return "TAB";
  case CppTokenizer::TokenType::SEMICOLON:
    return "SEMICOLON";
  case CppTokenizer::TokenType::COMMA:
    return "COMMA";
  case CppTokenizer::TokenType::ESCAPE_BACKSLASH:
    return "ESCAPE_BACKSLASH";
  case CppTokenizer::TokenType::BRACKET_OPEN:
    return "BRACKET_OPEN";
  case CppTokenizer::TokenType::BRACKET_CLOSE:
    return "BRACKET_CLOSE";
  case CppTokenizer::TokenType::SQUARE_BRACKET_OPEN:
    return "SQUARE_BRACKET_OPEN";
  case CppTokenizer::TokenType::SQUARE_BRACKET_CLOSE:
    return "SQUARE_BRACKET_CLOSE";
  case CppTokenizer::TokenType::CURLY_BRACE_OPEN:
    return "CURLY_BRACE_OPEN";
  case CppTokenizer::TokenType::CURLY_BRACE_CLOSE:
    return "CURLY_BRACE_CLOSE";
  case CppTokenizer::TokenType::CHARACTER:
    return "CHARACTER";
  case CppTokenizer::TokenType::STRING:
    return "STRING";
  case CppTokenizer::TokenType::COMMENT:
    return "COMMENT";
  case CppTokenizer::TokenType::MULTILINE_COMMENT:
    return "MULTILINE_COMMENT";
  case CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE:
    return "MULTILINE_COMMENT_INCOMPLETE";
  case CppTokenizer::TokenType::OPERATOR:
    return "OPERATOR";
  case CppTokenizer::TokenType::KEYWORD:
    return "KEYWORD";
  case CppTokenizer::TokenType::PREPROCESSOR_DIRECTIVE:
    return "PREPROCESSOR_DIRECTIVE";
  case CppTokenizer::TokenType::IDENTIFIER:
    return "IDENTIFIER";
  case CppTokenizer::TokenType::NUMBER:
    return "NUMBER";
  case CppTokenizer::TokenType::FUNCTION:
    return "FUNCTION";
  case CppTokenizer::TokenType::HEADER:
    return "HEADER";
  default:
    break;
  }

  return "UNKNOWN";
}

namespace CppTokenizer
{

Token::Token() noexcept
  : type(TokenType::UNKNOWN), start_offset(0), end_offset(0), value("")
{}

Token::Token(const TokenType& token_type) noexcept
  : type(token_type), start_offset(0), end_offset(0), value("")
{}

bool Token::operator!=(const Token& other) const noexcept
{
  return type != other.type || start_offset != other.start_offset ||
         end_offset != other.end_offset || value != other.value;
}

Tokenizer::Tokenizer() noexcept
  : _current_token(Token())
  , _tokens({})
  , _inside_char(false)
  , _inside_string(false)
  , _inside_comment(false)
  , _inside_multiline_comment(false)
  , _position(0)
{}

const std::vector<Token>& Tokenizer::tokenize(const std::string& str) noexcept
{
  while(_position < str.size())
  {
    char character = str[_position];

    /// Preprocessor Directive
    if(character == '#')
    {
      // mathcing with preprocessor directives
      for(const std::string directive : preprocessor_directives)
      {
        if(str.compare(_position, directive.size(), directive) == 0)
        {
          // found directive match
          _current_token = Token(TokenType::PREPROCESSOR_DIRECTIVE);
          _current_token.start_offset = _position;
          _current_token.end_offset = _position + directive.size() - 1;
          _current_token.value = directive;
          _tokens.emplace_back(_current_token);
          _position += directive.size();
          goto while_loop_continue;
        }
      }
    }

    /// Separator
    if(seperators.find(character) != std::string::npos)
    {
      if(character == '\'')
      {
        if(_inside_char)
        {
          _inside_char = false;
          _current_token.value.push_back(character);
          _current_token.end_offset = _position;
          _tokens.emplace_back(_current_token);
          _position++;
          continue;
        }
        _inside_char = true;
        _current_token = Token(TokenType::CHARACTER);
        _current_token.value.push_back(character);
        _current_token.start_offset = _position;
        _position++;
        continue;
      }
      else if(character == '"')
      {
        if(this->inside_include_declaration())
        {
          _current_token = Token(TokenType::HEADER);
          _current_token.start_offset = _position;
          _current_token.value.push_back(str[_position]);
          _position++;
          // move forward until we encounter '"'
          std::string header_string_separators =
            " \n\r!\t;:+*&%<>=(){}[]\"',|~^";
          while(_position < str.size() &&
                header_string_separators.find(str[_position]) ==
                  std::string::npos)
          {
            _current_token.value.push_back(str[_position]);
            _position++;
          }
          if(str[_position] == '"')
          {
            _current_token.value.push_back(str[_position]);
            _position++;
          }
          _current_token.end_offset = _position - 1;
          _tokens.emplace_back(_current_token);
          goto while_loop_continue;
        }

        // it is just a string, move forward until u find a string separator
        _current_token = Token(TokenType::STRING);
        _current_token.start_offset = _position;
        _current_token.value.push_back('"');
        _position++;
        while(_position < str.size() && str[_position] != '\n' &&
              str[_position] != '\t' && str[_position] != '\r' &&
              str[_position] != '"')
        {
          _current_token.value.push_back(str[_position]);
          _position++;
        }
        if(str[_position] == '"')
        {
          _current_token.value.push_back(str[_position]);
          _position++;
        }
        _current_token.end_offset = _position - 1;
        _tokens.emplace_back(_current_token);
        goto while_loop_continue;
      }
      else if(character == '<')
      {
        // check if it is include declaration
        if(this->inside_include_declaration())
        {
          _current_token = Token(TokenType::HEADER);
          _current_token.start_offset = _position;
          // move forward until we encounter '>'
          while(_position < str.size() && str[_position] != '>' &&
                str[_position] != '\n' && str[_position] != '\t' &&
                str[_position] != '\r' && str[_position] != ' ')
          {
            _current_token.value.push_back(str[_position]);
            _position++;
          }
          if(str[_position] == '>')
          {
            _current_token.value.push_back(str[_position]);
            _position++;
          }
          _current_token.end_offset = _position - 1;
          _tokens.emplace_back(_current_token);
          goto while_loop_continue;
        }
        // else consider it as operator
      }
      else if(character == '/')
      {
        if(_inside_multiline_comment)
        {
          if(_current_token.value.back() == '*')
          {
            _inside_multiline_comment = false;
            _current_token.end_offset = _position;
            _current_token.value.push_back(character);
            _tokens.emplace_back(_current_token);
            _position++;
            goto while_loop_continue;
          }
          _current_token.value.push_back(character);
          _position++;
          goto while_loop_continue;
        }
        if(_position + 1 < str.size() && str[_position + 1] == '/')
        {
          // single line comment
          _current_token = Token(TokenType::COMMENT);
          _current_token.start_offset = _position;
          while(_position < str.size() && str[_position] != '\r' &&
                str[_position] != '\n')
          {
            _current_token.value.push_back(str[_position]);
            _position++;
          }
          _current_token.end_offset = _position - 1;
          _tokens.push_back(_current_token);
          goto while_loop_continue;
        }
        if(_position + 1 < str.size() && str[_position + 1] == '*')
        {
          // multiline comment
          _current_token = Token(TokenType::MULTILINE_COMMENT);
          _current_token.start_offset = _position;
          _current_token.value = "/*";
          _position += 2;
          bool inserted_multiline_comment_token = false;
          while(_position < str.size())
          {
            if(str[_position] == '*' && _position + 1 < str.size() &&
               str[_position + 1] == '/')
            {
              // multiline comment ended
              _current_token.value.append("*/");
              _position += 2;
              _current_token.end_offset = _position - 1;
              _tokens.push_back(_current_token);
              inserted_multiline_comment_token = true;
              break;
            }
            if(str[_position] == '\r')
            {
              // skipping these characters for now
              // as this is geared towards syntax highlighting
              // while rendering these characters there will be glitches
              _position++;
              continue;
            }
            _current_token.value.push_back(str[_position]);
            _position++;
          }
          if(_position >= str.size() && !inserted_multiline_comment_token)
          {
            // end of string
            // although the miltiline comment is unclosed,
            // with token as incomplete type
            _current_token.type = TokenType::MULTILINE_COMMENT_INCOMPLETE;
            _current_token.end_offset = _position - 1;
            _tokens.push_back(_current_token);
          }
          goto while_loop_continue;
        }
      }
      else if(character == '\n')
      {
        if(_inside_comment)
        {
          _inside_comment = false;
          _current_token.end_offset = _position - 1;
          _tokens.emplace_back(_current_token);
          _position++;
          goto while_loop_continue;
        }
        if(_inside_multiline_comment || _inside_string)
        {
          _current_token.value.push_back(character);
          _position++;
          goto while_loop_continue;
        }
        if(_current_token.type == TokenType::IDENTIFIER &&
           _tokens.back() != _current_token)
        {
          _current_token.end_offset = _position - 1;
          _tokens.emplace_back(_current_token);
          _position++;
          goto while_loop_continue;
        }
        // ignore
        _position++;
        goto while_loop_continue;
      }
      else if(character == '\r')
      {
        // ignore carriage return characters
        _position++;
        goto while_loop_continue;
      }
      else if(character == ' ')
      {
        if(_inside_comment || _inside_multiline_comment || _inside_char || _inside_string)
        {
          _current_token.value.push_back(character);
          _position++;
          goto while_loop_continue;
        }
        _current_token = Token(TokenType::WHITESPACE);
        _current_token.start_offset = _position;
        _current_token.end_offset = _position;
        _current_token.value.push_back(character);
        _tokens.emplace_back(_current_token);
        _position++;
        goto while_loop_continue;
      }
      else if(character == '\t')
      {
        if(_inside_comment || _inside_multiline_comment || _inside_string)
        {
          _current_token.value.push_back(character);
          _position++;
          goto while_loop_continue;
        }

        _current_token = Token(TokenType::TAB);
        _current_token.start_offset = _position;
        _current_token.end_offset = _position;
        _current_token.value.push_back(character);
        _tokens.emplace_back(_current_token);
        _position++;
        goto while_loop_continue;
      }
      else if(character == '.')
      {
        if(_inside_comment || _inside_multiline_comment || _inside_char ||
           _inside_string)
        {
          _current_token.value.push_back(character);
          _position++;
          goto while_loop_continue;
        }
      }
      else if(character == ',')
      {
        if(_inside_comment || _inside_multiline_comment || _inside_char ||
           _inside_string)
        {
          _current_token.value.push_back(character);
          _position++;
          goto while_loop_continue;
        }
        _current_token = Token(TokenType::COMMA);
        _current_token.start_offset = _position;
        _current_token.end_offset = _position;
        _current_token.value.push_back(character);
        _tokens.emplace_back(_current_token);
        _position++;
        goto while_loop_continue;
      }
      else if(character == ';')
      {
        if(_inside_comment || _inside_multiline_comment || _inside_char ||
           _inside_string)
        {
          _current_token.value.push_back(character);
          _position++;
          goto while_loop_continue;
        }
        _current_token = Token(TokenType::SEMICOLON);
        _current_token.start_offset = _position;
        _current_token.end_offset = _position;
        _current_token.value.push_back(character);
        _tokens.emplace_back(_current_token);
        _position++;
        goto while_loop_continue;
      }
      else if(character == '(')
      {
        if(_inside_comment || _inside_multiline_comment || _inside_char ||
           _inside_string)
        {
          _current_token.value.push_back(character);
          _position++;
          goto while_loop_continue;
        }
        if(_tokens.back().type == TokenType::IDENTIFIER)
        {
          // this identifier should definitely be function
          _tokens.back().type = TokenType::FUNCTION;
        }
        _current_token = Token(TokenType::BRACKET_OPEN);
        _current_token.start_offset = _position;
        _current_token.end_offset = _position;
        _current_token.value.push_back(character);
        _tokens.emplace_back(_current_token);
        _position++;
        goto while_loop_continue;
      }
      else if(character == ')')
      {
        if(_inside_comment || _inside_multiline_comment || _inside_char ||
           _inside_string)
        {
          _current_token.value.push_back(character);
          _position++;
          goto while_loop_continue;
        }
        _current_token = Token(TokenType::BRACKET_CLOSE);
        _current_token.start_offset = _position;
        _current_token.end_offset = _position;
        _current_token.value.push_back(character);
        _tokens.emplace_back(_current_token);
        _position++;
        goto while_loop_continue;
      }
      else if(character == '[')
      {
        if(_inside_comment || _inside_multiline_comment || _inside_char ||
           _inside_string)
        {
          _current_token.value.push_back(character);
          _position++;
          goto while_loop_continue;
        }
        _current_token = Token(TokenType::SQUARE_BRACKET_OPEN);
        _current_token.start_offset = _position;
        _current_token.end_offset = _position;
        _current_token.value.push_back(character);
        _tokens.emplace_back(_current_token);
        _position++;
        goto while_loop_continue;
      }
      else if(character == ']')
      {
        if(_inside_comment || _inside_multiline_comment || _inside_char ||
           _inside_string)
        {
          _current_token.value.push_back(character);
          _position++;
          goto while_loop_continue;
        }
        _current_token = Token(TokenType::SQUARE_BRACKET_CLOSE);
        _current_token.start_offset = _position;
        _current_token.end_offset = _position;
        _current_token.value.push_back(character);
        _tokens.emplace_back(_current_token);
        _position++;
        goto while_loop_continue;
      }
      else if(character == '{')
      {
        if(_inside_comment || _inside_multiline_comment || _inside_char ||
           _inside_string)
        {
          _current_token.value.push_back(character);
          _position++;
          goto while_loop_continue;
        }
        _current_token = Token(TokenType::CURLY_BRACE_OPEN);
        _current_token.start_offset = _position;
        _current_token.end_offset = _position;
        _current_token.value.push_back(character);
        _tokens.emplace_back(_current_token);
        _position++;
        goto while_loop_continue;
      }
      else if(character == '}')
      {
        if(_inside_comment || _inside_multiline_comment || _inside_char ||
           _inside_string)
        {
          _current_token.value.push_back(character);
          _position++;
          goto while_loop_continue;
        }
        _current_token = Token(TokenType::CURLY_BRACE_CLOSE);
        _current_token.start_offset = _position;
        _current_token.end_offset = _position;
        _current_token.value.push_back(character);
        _tokens.emplace_back(_current_token);
        _position++;
        goto while_loop_continue;
      }
    }

    /// continuing if inside comment
    if(_inside_comment)
    {
      while(_position < str.size() && str[_position] != '\n')
      {
        _current_token.value.push_back(str[_position]);
        _position++;
      }
      goto while_loop_continue;
    }

    /// continuing if inside multiline comment
    if(_inside_multiline_comment)
    {
      while(_position < str.size() && str[_position] != '*')
      {
        _current_token.value.push_back(str[_position]);
        _position++;
      }
      goto while_loop_continue;
    }

    /// Operator
    for(const std::string& op : operators)
    {
      if(str.compare(_position, op.size(), op) == 0)
      {
        // found operator match
        _current_token = Token(TokenType::OPERATOR);
        _current_token.start_offset = _position;
        _current_token.end_offset = _position + op.size() - 1;
        _current_token.value = op;
        _tokens.emplace_back(_current_token);
        _position += op.size();
        goto while_loop_continue;
      }
    }

    /// Keyword
    for(const std::string keyword : keywords)
    {
      if(str.compare(_position, keyword.size(), keyword) == 0)
      {
        if(_position + keyword.size() != str.size() &&
           seperators.find(str[_position + keyword.size()]) ==
             std::string::npos)
        {
          // next character after this keyword is not separator
          // so this must be an indentifier
          break;
        }
        // found keyword match
        _current_token = Token(TokenType::KEYWORD);
        _current_token.start_offset = _position;
        _current_token.end_offset = _position + keyword.size() - 1;
        _current_token.value = keyword;
        _tokens.emplace_back(_current_token);
        _position += keyword.size();
        goto while_loop_continue;
      }
    }

    /// Alphabets
    if(_inside_comment || _inside_multiline_comment || _inside_string ||
       _inside_char)
    {
      _current_token.value.push_back(character);
      _position++;
      goto while_loop_continue;
    }
    if(isdigit(character))
    {
      // character is number - '0' to '9'
      _current_token = Token(TokenType::NUMBER);
      _current_token.start_offset = _position;
      while(_position < str.size())
      {
        if(isdigit(str[_position]) || str[_position] == '.' ||
           str[_position] == 'e' || str[_position] == 'f' ||
           (str[_position] == '-' && _current_token.value.back() == 'e'))
        {
          _current_token.value.push_back(str[_position]);
          _position++;
          continue;
        }
        break;
      }
      _current_token.end_offset = _position - 1;
      _tokens.emplace_back(_current_token);
      goto while_loop_continue;
    }
    _current_token = Token(TokenType::IDENTIFIER);
    _current_token.start_offset = _position;
    _current_token.value = character;
    _position++;
    // moving forward until a separator is encountered
    while(_position < str.size() &&
          seperators.find(str[_position]) == std::string::npos)
    {
      _current_token.value.push_back(str[_position]);
      _position++;
    }
    _current_token.end_offset = _position - 1;
    _tokens.emplace_back(_current_token);

  while_loop_continue:
    continue;
  }

  return _tokens;
}

const std::vector<Token>& Tokenizer::tokenize_from_imcomplete_token(
  const std::string& str, const Token& incomplete_token) noexcept
{
  if(incomplete_token.type == TokenType::MULTILINE_COMMENT_INCOMPLETE)
  {
    _current_token = Token(TokenType::MULTILINE_COMMENT_INCOMPLETE);
    bool inserted_multiline_comment_token = false;
    while(_position < str.size())
    {
      if(str[_position] == '*' && _position + 1 < str.size() &&
         str[_position + 1] == '/')
      {
        // multiline comment ended.
        _current_token.value.append("*/");
        _current_token.end_offset = _position + 1;
        _position += 2;
        _current_token.type = TokenType::MULTILINE_COMMENT;
        _tokens.push_back(_current_token);
        inserted_multiline_comment_token = true;
        break;
      }
      if(str[_position] == '\r')
      {
        _position++;
        continue;
      }
      _current_token.value.push_back(str[_position]);
      _position++;
    }
    if(_position >= str.size() && !inserted_multiline_comment_token)
    {
      _current_token.end_offset = _position - 1;
      _tokens.push_back(_current_token);
      return _tokens;
    }
  }

  return this->tokenize(str);
}

void Tokenizer::clear_tokens() noexcept
{
  _inside_char = false;
  _inside_string = false;
  _inside_comment = false;
  _inside_multiline_comment = false;
  _position = 0;
  _current_token = Token();
  _tokens.clear();
}

bool Tokenizer::inside_include_declaration() const noexcept
{
  // first token backwards which is not whitespace
  // if it is #include, it is include declaration
  for(auto it = _tokens.rbegin(); it != _tokens.rend(); it++)
  {
    if(it->type != TokenType::WHITESPACE)
    {
      if(it->type == TokenType::PREPROCESSOR_DIRECTIVE &&
         it->value == "#include")
      {
        return true;
      }
      break;
    }
  }
  return false;
}

void log_tokens(const std::vector<Token>& tokens) noexcept
{
  printf("Tokens: [");
  for(uint32_t i = 0; i < tokens.size(); i++)
  {
    Token token = tokens[i];
    if(i != tokens.size() - 1)
    {
      printf("\n  {\n    type: %s,\n    start_offset: %lu,\n    end_offset: "
             "%lu,\n    value: \"%s\"\n  },",
             token_type_to_string(token.type).c_str(),
             (unsigned long)token.start_offset,
             (unsigned long)token.end_offset,
             token.value.c_str());
    }
    else
    {
      printf("\n  {\n    type: %s,\n    start_offset: %lu,\n    end_offset: "
             "%lu,\n    value: \"%s\"\n  }",
             token_type_to_string(token.type).c_str(),
             (unsigned long)token.start_offset,
             (unsigned long)token.end_offset,
             token.value.c_str());
    }
  }
  printf("\n]\n");
}

}; // namespace CppTokenizer
