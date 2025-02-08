#pragma once
#include <string>
#include <map>

namespace kal {

enum Token {
  END_OF_FILE = -1,
  DEF = -2,
  EXTERN = -3,
  IDENTIFIER =-4,
  NUMBER=-5,
  IF = -6,
  THEN = -7,
  ELSE = -8,
  FOR = -9,
  IN = -10,
  BINARY = -11,
  UNARY  = -12,
  VARIABLE = -13,

  UNKNOWN = INT32_MAX
};


struct SourceLocation
{
  int m_line;
  int m_col;
};

class Tokenizer {
public:

  static int  get_token();
  static int  get_next_token();
  static int  get_token_precedence();
  static void init_tokenizer_presedence();

  inline static std::string           s_identifier_str;
  inline static double                s_number_value;
  inline static int                   s_current_token = 0;
  inline static std::map<char, int>   s_op_precedence;

  inline static SourceLocation                                m_debug_current_loc;
  inline static SourceLocation                                m_debug_lexer_loc;

  static int advance();
};
} // namespace turas