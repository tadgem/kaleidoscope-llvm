#pragma once
#include "defines.h"

namespace kal {

enum Token {
  END_OF_FILE = 0,
  DEF,
  EXTERN,
  IDENTIFIER,
  NUMBER,

  UNKNOWN = INT32_MAX
};

class Tokenizer {
public:

  static Token get_token();

  inline static String    s_identifier_str;
  inline static double    s_number_value;

};
} // namespace turas