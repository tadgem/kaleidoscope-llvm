#include "token.h"

int kal::Tokenizer::get_token()
{
  using namespace kal;

  static int last_char = ' ';
  while(isspace(last_char))
  {
    last_char = getchar();
  }

  if(isalpha(last_char))
  {
    s_identifier_str = last_char;
    while (isalnum((last_char = getchar()))) {
      s_identifier_str += last_char;
    }

    if(s_identifier_str == "def")
    {
      return Token::DEF;
    }

    if(s_identifier_str == "extern")
    {
      return Token::EXTERN;
    }

    if(s_identifier_str == "if")
    {
      return Token::IF;
    }

    if(s_identifier_str == "then")
    {
      return Token::THEN;
    }

    if(s_identifier_str == "else")
    {
      return Token::ELSE;
    }

    if(s_identifier_str == "for")
    {
      return Token::FOR;
    }

    if(s_identifier_str == "in")
    {
      return Token::IN;
    }

    if(s_identifier_str == "binary")
    {
      return Token::BINARY;
    }

    if(s_identifier_str == "unary")
    {
      return Token::UNARY;
    }

    return Token::IDENTIFIER;
  }

  if(isdigit(last_char) || last_char == '.')
  {
    std::string num_str;

    while(isdigit(last_char) || last_char == '.')
    {
      num_str += last_char;
      last_char = getchar();
    }
    s_number_value = strtod(num_str.c_str(), 0);
    return Token::NUMBER;
  }

  if(last_char == '#')
  {
    while(last_char != EOF && last_char != '\n' && last_char != '\r')
    {
      last_char = getchar();
    }
    if(last_char != EOF)
    {
      return get_token();
    }
  }

  if(last_char == EOF)
  {
    return Token::END_OF_FILE;
  }

  int this_char = last_char;
  last_char = getchar();
  return this_char;
}
int kal::Tokenizer::get_next_token() {
  return s_current_token = get_token();
}
void kal::Tokenizer::init_tokenizer_presedence() {
  s_op_precedence['='] = 2;
  s_op_precedence['<'] = 10;
  s_op_precedence['+'] = 20;
  s_op_precedence['-'] = 20;
  s_op_precedence['*'] = 40;
}
int kal::Tokenizer::get_token_precedence() {
  if(!isascii(s_current_token))
  {
    return -1;
  }

  int token_precedence = s_op_precedence[s_current_token];

  if(token_precedence <= 0) return -1;

  return token_precedence;

}
