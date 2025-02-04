#include "helpers.h"
#include "generate.h"
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
llvm::Function *kal::Helpers::GetFunction(std::string name) {
  if(auto* f = Generator::m_module->getFunction(name))
  {
    return f;
  }

  auto fi = Generator::m_function_protos.find(name);
  if(fi != Generator::m_function_protos.end())
  {
    return fi->second->codegen();
  }

  return nullptr;
}
