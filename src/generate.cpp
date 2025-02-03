#include "generate.h"
void kal::Generator::init_generator() {
//  static std::unique_ptr<LLVMContext>             m_context;
//  static std::unique_ptr<IRBuilder<>>             m_ir_builder;
//  static std::unique_ptr<Module>                  m_module;
//  static std::unordered_map<std::string, Value*>  m_named_values;

  m_context = std::make_unique<llvm::LLVMContext>();
  m_ir_builder = std::make_unique<llvm::IRBuilder<>>(*m_context, llvm::ConstantFolder());
  m_module = std::make_unique<llvm::Module>("test_module", *m_context);
}
