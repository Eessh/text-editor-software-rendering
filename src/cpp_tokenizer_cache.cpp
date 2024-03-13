#include "../include/cpp_tokenizer_cache.hpp"
#include <algorithm>
#include "../include/buffer.hpp"
#include "../include/incremental_render_update.hpp"
#include "../include/macros.hpp"

void CppTokenizerCache::build_cache(const Buffer& buffer) noexcept
{
  _tokens.clear();
  for(uint32 i = 0; i < buffer.length(); i++)
  {
    if(!_tokens.empty() && !_tokens.back().empty() &&
       _tokens.back().back().type ==
         CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE)
    {
      _tokens.push_back(_tokenizer.tokenize_from_imcomplete_token(
        buffer.line(i).value().get(), _tokens.back().back()));
    }
    else
    {
      _tokens.push_back(_tokenizer.tokenize(
        buffer.line_with_spaces_converted_to_tabs(i).value()));
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
      if(row != 0 && !_tokens[row - 1].empty() &&
         _tokens[row - 1].back().type ==
           CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE)
      {
        // before line is an incomplete multiline comment
        // so either this line is still a incomplete multiline comment
        // or multiline comment ends in this line (as it is edited)
        std::vector<CppTokenizer::Token> tokens_ =
          _tokenizer.tokenize_from_imcomplete_token(
            buffer.line(row).value().get(), _tokens[row - 1].back());
        {
          uint32 i = 0;
          //          while(i < std::min(tokens_.size(), _tokens[row].size()))
          //          {
          //            if(tokens_[i] != _tokens[row][i])
          //            {
          //              IncrementalRenderUpdateCommand cmd;
          //              cmd.type = IncrementalRenderUpdateType::RENDER_LINE_SLICE;
          //              cmd.row_start = row;
          //              cmd.token_start = i;
          //              cmd.token_end = tokens_.size() - 1;
          //              _incremental_render_updates_queue.emplace_back(cmd);
          //              break;
          //            }
          //            i++;
          //          }
          //          // new token is added to pervious tokens
          //          if(i == std::min(tokens_.size(), _tokens[row].size()))
          //          {
          //            IncrementalRenderUpdateCommand cmd;
          //            cmd.type = IncrementalRenderUpdateType::RENDER_LINE_SLICE;
          //            cmd.row = row;
          //            cmd.token_start = i;
          //            cmd.token_end = tokens_.size() - 1;
          //            _incremental_render_updates_queue.emplace_back(cmd);
          //          }
        }
        _tokens[row] = tokens_;
        _tokenizer.clear_tokens();
        _re_tokenized_lines.push_back(row);
        if(!_tokens.empty() &&
           _tokens[row].back().type !=
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
              _tokens[next_row] = _tokenizer.tokenize(
                buffer.line_with_spaces_converted_to_tabs(next_row).value());
              {
                IncrementalRenderUpdateCommand cmd;
                cmd.type = IncrementalRenderUpdateType::RENDER_LINE;
                cmd.row_start = next_row;
                _incremental_render_updates_queue.emplace_back(cmd);
              }
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
        std::vector<CppTokenizer::Token> tokens_ = _tokenizer.tokenize(
          buffer.line_with_spaces_converted_to_tabs(row).value());
        {
          uint32 i = 0;
          //          while(i < std::min(tokens_.size(), _tokens[row].size()))
          //          {
          //            if(tokens_[i] != _tokens[row][i])
          //            {
          //              IncrementalRenderUpdateCommand cmd;
          //              cmd.type = IncrementalRenderUpdateType::RENDER_LINE_SLICE;
          //              cmd.row = row;
          //              cmd.token_start = i;
          //              cmd.token_end = tokens_.size() - 1;
          //              _incremental_render_updates_queue.emplace_back(cmd);
          //              break;
          //            }
          //            i++;
          //          }
          //          // new token is added to pervious tokens
          //          if(i == std::min(tokens_.size(), _tokens[row].size()))
          //          {
          //            IncrementalRenderUpdateCommand cmd;
          //            cmd.type = IncrementalRenderUpdateType::RENDER_LINE_SLICE;
          //            cmd.row = row;
          //            cmd.token_start = i;
          //            cmd.token_end = tokens_.size() - 1;
          //            _incremental_render_updates_queue.emplace_back(cmd);
          //          }
        }
        _tokens[row] = tokens_;
        _tokenizer.clear_tokens();
        _re_tokenized_lines.push_back(row);

        if(!_tokens.empty() && !_tokens[row].empty() &&
           _tokens[row].back().type ==
             CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE)
        {
          // hekk it! we now need to re-tokenize all lines
          // below this line as multiline comment
          uint32 next_row = row + 1;
          while(next_row < buffer.length())
          {
            _tokens[next_row] = _tokenizer.tokenize(
              buffer.line_with_spaces_converted_to_tabs(next_row).value());
            {
              IncrementalRenderUpdateCommand cmd;
              cmd.type = IncrementalRenderUpdateType::RENDER_LINE;
              cmd.row_start = next_row;
              _incremental_render_updates_queue.emplace_back(cmd);
            }
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
      // re-tokenize line if its length is changed
      uint32 line_length = 0;
      for(auto it = _tokens[command.row].cbegin();
          it != _tokens[command.row].cend();
          it++)
      {
        line_length += it->value.size();
      }
      if(line_length != buffer.line_length(command.row).value())
      {
        // find nearest word to tokenize from
        // then re-tokenize from it
        uint32 token_index = _tokens[command.row].size() - 1;
        while(token_index != 0 &&
              line_length > buffer.line_length(command.row).value())
        {
          line_length -= _tokens[command.row][token_index].value.size();
          token_index--;
        }

        /// FIXME: maybe be possibly missed some edge cases

        if(token_index == 0)
        {
          // just re-tokenize it
          _tokens[command.row] = _tokenizer.tokenize(
            buffer.line_with_spaces_converted_to_tabs(command.row).value());
          _tokenizer.clear_tokens();
          IncrementalRenderUpdateCommand cmd;
          cmd.type = IncrementalRenderUpdateType::RENDER_LINE;
          cmd.row_start = command.row;
          _incremental_render_updates_queue.emplace_back(cmd);
        }
        //        else
        //        {
        //          auto tokens_ = _tokenizer.tokenize(
        //            buffer.line_with_spaces_converted_to_tabs(command.row)
        //              .value()
        //              .substr(line_length));
        //          _tokenizer.clear_tokens();
        //          _tokens[command.row].insert(
        //            _tokens[command.row].end(), tokens_.begin(), tokens_.end());
        //          IncrementalRenderUpdateCommand cmd;
        //          cmd.type = IncrementalRenderUpdateType::RENDER_LINE_SLICE;
        //          cmd.row = command.row;
        //          cmd.token_start = token_index + 1;
        //          cmd.token_end = _tokens[command.row].size() - 1;
        //          _incremental_render_updates_queue.emplace_back(cmd);
        //        }
      }
      _tokens.insert(_tokens.begin() + command.row + 1,
                     std::vector<CppTokenizer::Token>());
      if(!_tokens[command.row].empty() &&
         _tokens[command.row].back().type ==
           CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE)
      {
        _tokens[command.row + 1] = _tokenizer.tokenize_from_imcomplete_token(
          buffer.line(command.row + 1).value().get(),
          _tokens[command.row].back());
      }
      else
      {
        _tokens[command.row + 1] = _tokenizer.tokenize(
          buffer.line_with_spaces_converted_to_tabs(command.row + 1).value());
      }
      _tokenizer.clear_tokens();
      //      IncrementalRenderUpdateCommand cmd;
      //      cmd.type = IncrementalRenderUpdateType::RENDER_LINES_FROM;
      //      cmd.row_start = command.row + 1;
      //      _incremental_render_updates_queue.emplace_back(cmd);
    }
    else if(command.type == TokenCacheUpdateCommandType::DELETE_LINE_CACHE)
    {
      if(!_tokens.empty() && command.row != 0 &&
         !_tokens[command.row - 1].empty() &&
         _tokens[command.row - 1].back().type ==
           CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE &&
         !_tokens[command.row].empty() &&
         _tokens[command.row].back().type !=
           CppTokenizer::TokenType::MULTILINE_COMMENT_INCOMPLETE)
      {
        // deleted line consists the closing (*/) of multiline comment
        // we should include all of lines next to this line in multline comment
        _tokens.erase(_tokens.begin() + command.row);
        uint32 next_row = command.row;
        while(next_row < _tokens.size())
        {
          _tokens[next_row] = _tokenizer.tokenize_from_imcomplete_token(
            buffer.line(next_row).value().get(), _tokens[next_row - 1].back());
          {
            IncrementalRenderUpdateCommand cmd;
            cmd.type = IncrementalRenderUpdateType::RENDER_LINE;
            cmd.row_start = next_row;
            _incremental_render_updates_queue.emplace_back(cmd);
          }
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
      else
      {
        // just delete the line
        _tokens.erase(_tokens.begin() + command.row);
        //        if(command.row < _tokens.size())
        //        {
        //          IncrementalRenderUpdateCommand cmd;
        //          cmd.type = IncrementalRenderUpdateType::RENDER_LINES_FROM;
        //          cmd.row = command.row;
        //          _incremental_render_updates_queue.emplace_back(cmd);
        //        }
      }
    }
    else
    {
      // TokenCacheUpdateCommandType::DELETE_LINES_CACHE

      /// TODO: handle multiline comment shit

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

const std::vector<CppTokenizer::Token>*
CppTokenizerCache::tokens_for_line(const uint32& line_index) const noexcept
{
  if(line_index >= _tokens.size())
  {
    return nullptr;
  }

  return &_tokens[line_index];
}

std::optional<IncrementalRenderUpdateCommand>
CppTokenizerCache::get_next_incremental_render_update() noexcept
{
  if(_incremental_render_updates_queue.empty())
  {
    return std::nullopt;
  }

  auto cmd = _incremental_render_updates_queue.front();
  _incremental_render_updates_queue.pop_front();
  return cmd;
}
