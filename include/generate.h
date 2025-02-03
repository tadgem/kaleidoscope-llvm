#pragma once
#include <memory>
#include <map>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/SandboxIR/Module.h"

using namespace llvm;

namespace kal {
class Generator {
public:

  static void init_generator();

  inline static std::unique_ptr<LLVMContext>             m_context;
  inline static std::unique_ptr<IRBuilder<>>             m_ir_builder;
  inline static std::unique_ptr<Module>                  m_module;
  inline static std::unordered_map<std::string, Value*>  m_named_values;
};
}