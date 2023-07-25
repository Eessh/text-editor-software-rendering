#include "../include/cpp_tokenizer_cache.hpp"
#include <algorithm>

void CppTokenizerCache::build_cache(const Buffer& buffer) noexcept
{
  for(const std::string& line : buffer.lines())
  {
    if(!_tokens.empty() && !_tokens.back().empty() &&
       _tokens.back().back().type ==
         CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE)
    {
      _tokens.push_back(
        _tokenizer.tokenize_from_imcomplete_token(line, _tokens.back().back()));
    }
    else
    {
      _tokens.push_back(_tokenizer.tokenize(line));
    }
    _tokenizer.clear_tokens();
  }
}

void CppTokenizerCache::update_cache(Buffer& buffer) noexcept
{
  // clearing re-tokenized lines in last cache update
  _re_tokenized_lines.clear();

  auto cmd = buffer.get_next_token_cache_update_command();
  while(cmd != std::nullopt)
  {
    TokenCacheUpdateCommand command = cmd.value();
    if(command.type == TokenCacheUpdateCommandType::RETOKENIZE_LINE)
    {
      uint32 row = command.row;
      if(row - 1 >= 0 &&
         _tokens[row - 1].back().type ==
           CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE)
      {
        // before line is an incomplete multiline comment
        // so either this line is still a incomplete multiline comment
        // or multiline comment ends in this line (as it is edited)
        _tokens[row] = _tokenizer.tokenize_from_imcomplete_token(
          buffer.line(row).value().get(), _tokens[row - 1].back());
        _tokenizer.clear_tokens();
        _re_tokenized_lines.push_back(row);
        if(_tokens[row].back().type !=
           CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE)
        {
          // after this line is edited, multiline comment ended here
          // so lines after this should be retokenized till we encounter
          // a line which is not incomplettely tokenized.
          uint32 next_row = row + 1;
          while(next_row < buffer.length())
          {
            if(_tokens[next_row].back().type ==
               CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE)
            {
              _tokens[next_row] =
                _tokenizer.tokenize(buffer.line(next_row).value().get());
              _tokenizer.clear_tokens();
              _re_tokenized_lines.push_back(next_row);
            }
            else
            {
              break;
            }
            next_row++;
          }
        }
      }
      else
      {
        // as previous line is not incompletely tokenized
        // we re-tokenize this line normally
        _tokens[row] = _tokenizer.tokenize(buffer.line(row).value().get());
        _re_tokenized_lines.push_back(row);

        if(_tokens[row].back().type ==
           CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE)
        {
          // hekk it! we now need to re-tokenize all lines
          // below this line as multiline comment
          uint32 next_row = row + 1;
          while(next_row < buffer.length())
          {
            _tokens[next_row] =
              _tokenizer.tokenize(buffer.line(next_row).value().get());
            _tokenizer.clear_tokens();
            _re_tokenized_lines.push_back(next_row);
            if(_tokens[next_row].back().type !=
               CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE)
            {
              break;
            }
            next_row++;
          }
        }
      }
    }
    else if(command.type ==
            TokenCacheUpdateCommandType::INSERT_NEW_LINE_CACHE_AND_TOKENIZE)
    {
      _tokens.insert(_tokens.begin() + command.row + 1, {});
      if(_tokens[command.row].back().type ==
         CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE)
      {
        _tokens[command.row + 1] = _tokenizer.tokenize_from_imcomplete_token(
          buffer.line(command.row + 1).value().get(),
          _tokens[command.row].back());
      }
      else
      {
        _tokens[command.row + 1] =
          _tokenizer.tokenize(buffer.line(command.row + 1).value().get());
      }
      _tokenizer.clear_tokens();
    }
    else if(command.type == TokenCacheUpdateCommandType::DELETE_LINE_CACHE)
    {
      _tokens.erase(_tokens.begin() + command.row);
    }
    else
    {
      // TokenCacheUpdateCommandType::DELETE_LINES_CACHE
      _tokens.erase(_tokens.begin() + command.start_row,
                    _tokens.begin() + command.end_row + 1);
    }

    cmd = buffer.get_next_token_cache_update_command();
  }
}

const std::vector<std::vector<CppTokenizer::Token>>&
CppTokenizerCache::tokens() const noexcept
{
  return _tokens;
}