#include "token.h"

kal::Token kal::Tokenizer::get_token()
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
  return Token::UNKNOWN;
}
