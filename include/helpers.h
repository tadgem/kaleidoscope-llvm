#pragma once
#include "ast.h"

namespace kal {
class Helpers {
public:

  static std::unique_ptr<ExprAST>       LogErrorExpr(const char* msg);
  static std::unique_ptr<PrototypeAST>  LogErrorProrotype(const char* msg);
};
}