#pragma once
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/PassInstrumentation.h"
#include "llvm/SandboxIR/Module.h"
#include <llvm/IR/PassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <map>
#include <memory>

using namespace llvm;

namespace kal {
class PrototypeAST;

class Generator {
public:

  static void init_generator();
  static void init_opt_passes();

  inline static bool m_debug_logging = true;

  inline static std::unique_ptr<LLVMContext>                m_context;
  inline static std::unique_ptr<IRBuilder<>>                m_ir_builder;
  inline static std::unique_ptr<Module>                     m_module;
  inline static std::unordered_map<std::string, Value*>     m_named_values;
  inline static std::unordered_map<std::string,
      std::unique_ptr<PrototypeAST>>                        m_function_protos;

  // IR Optimzation passes
  inline static std::unique_ptr<FunctionPassManager>            m_FPM;
  inline static std::unique_ptr<FunctionAnalysisManager>        m_FAM;
  inline static std::unique_ptr<LoopAnalysisManager>            m_LAM;
  inline static std::unique_ptr<CGSCCAnalysisManager>           m_CGAM;
  inline static std::unique_ptr<ModuleAnalysisManager>          m_MAM;
  inline static std::unique_ptr<PassInstrumentationCallbacks>   m_PIC;
  inline static std::unique_ptr<StandardInstrumentations>       m_SI;

};
}