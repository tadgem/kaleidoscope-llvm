#include "target.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include <llvm/MC/TargetRegistry.h>

void kal::target::init_target() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  if(m_use_jit)
  {
    return;
  }
  m_target_triple = llvm::sys::getDefaultTargetTriple();

  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  std::string error;
  auto target = llvm::TargetRegistry::lookupTarget(m_target_triple, error);

  if (!target)
  {
    llvm::errs() << error;
    return;
  }

  auto cpu = "generic";
  auto features = "";

  llvm::TargetOptions opt;

  m_target_machine = target->createTargetMachine(
      m_target_triple, cpu, features, opt, llvm::Reloc::PIC_);
}
