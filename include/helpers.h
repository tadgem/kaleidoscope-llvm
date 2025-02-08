#pragma once
#include "ast.h"

namespace llvm
{
  class Value;
  class Function;
  class AllocaInst;
  class DISubroutineType;
}

namespace kal {
class Helpers {
public:

  static std::unique_ptr<ExprAST>       LogErrorExpr(const char* msg);
  static std::unique_ptr<PrototypeAST>  LogErrorProrotype(const char* msg);
  static llvm::Value*                   LogErrorValue(const char* msg);
  static llvm::Function*                GetFunction(std::string name);
  static llvm::AllocaInst*              CreateEntryBlockAlloca(
      llvm::Function* func, const std::string& var_name);
  static llvm::DISubroutineType*        CreateDebugFunctionType(int num_args);

};
}