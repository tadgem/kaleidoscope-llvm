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

  if(Generator::m_output_object_file)
  {
    m_module->setDataLayout(target::m_target_machine->createDataLayout());
    m_module->setTargetTriple(target::m_target_triple);
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