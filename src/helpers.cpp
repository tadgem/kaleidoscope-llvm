#include "helpers.h"
std::unique_ptr<kal::ExprAST> kal::Helpers::LogErrorExpr(const char *msg) {
  fprintf(stderr, "Error : %s\n", msg);
  return nullptr;
}
std::unique_ptr<kal::PrototypeAST> kal::Helpers::LogErrorProrotype(const char *msg) {
  LogErrorExpr(msg);
  return nullptr;
}
llvm::Value *kal::Helpers::LogErrorValue(const char *msg) {
  LogErrorExpr(msg);
  return nullptr;
}
