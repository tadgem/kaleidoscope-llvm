#include "generate.h"
#include "jit.h"
#include "ast.h"
#include "target.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include "llvm/Passes/PassBuilder.h"

void kal::Generator::init_generator() {
  m_context = std::make_unique<llvm::LLVMContext>();
  m_ir_builder = std::make_unique<llvm::IRBuilder<>>(*m_context, llvm::ConstantFolder());
  m_module = std::make_unique<llvm::Module>("Main Module", *m_context);
  DataLayout layout = jit::s_jit->getDataLayout();
  m_module->setDataLayout(layout);

  if(m_output_object_file)
  {
    m_module->setDataLayout(target::m_target_machine->createDataLayout());
    m_module->setTargetTriple(target::m_target_triple);
  }

  if(m_debug)
  {
    m_debug_builder = std::make_unique<DIBuilder>(*m_module);
    m_debug_info.m_compile_unit = m_debug_builder->createCompileUnit(
        dwarf::DW_LANG_C, m_debug_builder->createFile("fib.kl", "."),
        "Kaleidoscope compiler", false, "", 0);
  }
}
void kal::Generator::init_opt_passes() {
  m_FPM = std::make_unique<FunctionPassManager>();
  m_FAM = std::make_unique<FunctionAnalysisManager>();
  m_LAM = std::make_unique<LoopAnalysisManager>();
  m_CGAM = std::make_unique<CGSCCAnalysisManager>();
  m_MAM = std::make_unique<ModuleAnalysisManager>();
  m_PIC = std::make_unique<PassInstrumentationCallbacks>();
  m_SI = std::make_unique<StandardInstrumentations>(*m_context, m_debug);

  m_SI->registerCallbacks(*m_PIC, m_MAM.get());

  m_FPM->addPass(PromotePass());
  m_FPM->addPass(InstCombinePass());
  m_FPM->addPass(ReassociatePass());
  if(!Generator::m_debug) {

    m_FPM->addPass(GVNPass());
    m_FPM->addPass(SimplifyCFGPass());
  }
  PassBuilder pb;

  pb.registerModuleAnalyses(*m_MAM);
  pb.registerFunctionAnalyses(*m_FAM);
  pb.crossRegisterProxies(*m_LAM,
                          *m_FAM,
                          *m_CGAM,
                          *m_MAM);

}

DIType *kal::DebugInfo::get_double_type() {
  if(m_double_type)
  {
    return m_double_type;
  }

  m_double_type = Generator::m_debug_builder->createBasicType("double", 64, dwarf::DW_ATE_float);
  return m_double_type;
}

void kal::DebugInfo::emit_location(kal::ExprAST *ast) {
  if(!ast)
  {
    return Generator::m_ir_builder->SetCurrentDebugLocation(DebugLoc());
  }
  DIScope* scope;
  if(m_lexical_blocks.empty())
  {
    scope = Generator::m_debug_info.m_compile_unit;
  }
  else
  {
    scope = m_lexical_blocks.back();
  }
  Generator::m_ir_builder->SetCurrentDebugLocation(DILocation::get(
      scope->getContext(), ast->get_line(), ast->get_col(),scope));
}
