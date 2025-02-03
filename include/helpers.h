#pragma once
#include "ast.h"

namespace llvm
{
  class Value;
}

namespace kal {
class Helpers {
public:

  static std::unique_ptr<ExprAST>       LogErrorExpr(const char* msg);
  static std::unique_ptr<PrototypeAST>  LogErrorProrotype(const char* msg);
  static llvm::Value*                   LogErrorValue(const char* msg);
};
}