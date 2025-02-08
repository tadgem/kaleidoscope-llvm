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
llvm::AllocaInst *
kal::Helpers::CreateEntryBlockAlloca(llvm::Function *func,
                                     const std::string &var_name) {
  llvm::IRBuilder<> tmpb (&func->getEntryBlock(), func->getEntryBlock().begin());
  return tmpb.CreateAlloca(
      llvm::Type::getDoubleTy(*Generator::m_context), nullptr, var_name);
}
llvm::DISubroutineType* kal::Helpers::CreateDebugFunctionType(int num_args) {
  llvm::SmallVector<Metadata*, 8> element_types;
  DIType* double_type = Generator::m_debug_info.get_double_type();

  element_types.push_back(double_type);
  for(int i = 0; i < num_args; i++)
  {
    element_types.push_back(double_type);
  }
  return Generator::m_debug_builder->createSubroutineType(
      Generator::m_debug_builder->getOrCreateTypeArray(element_types)
      );
}
