#pragma once
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/ExecutorProcessControl.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/Shared/ExecutorSymbolDef.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"
#include <memory>
#include "generate.h"
using namespace llvm;
using namespace orc;
namespace kal {

class jit;


class jit {
private:
  std::unique_ptr<orc::ExecutionSession>  p_ES;
  DataLayout                              p_data_layout;
  MangleAndInterner                  p_mangle;
  RTDyldObjectLinkingLayer           p_object_layer;
  IRCompileLayer                     p_compile_layer;
  JITDylib&                          p_main_jd;

public:
  inline static std::unique_ptr<jit> s_jit;

  jit(std::unique_ptr<ExecutionSession> ES,
                  JITTargetMachineBuilder JTMB, DataLayout DL)
      : p_ES(std::move(ES)), p_data_layout(DL), p_mangle(*this->p_ES, this->p_data_layout),
        p_object_layer(*this->p_ES,
                    []() { return std::make_unique<SectionMemoryManager>(); }),
        p_compile_layer(*this->p_ES, p_object_layer,
                     std::make_unique<ConcurrentIRCompiler>(std::move(JTMB))),
        p_main_jd(this->p_ES->createBareJITDylib("<main>")) {
    p_main_jd.addGenerator(
        cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(
            DL.getGlobalPrefix())));
    if (JTMB.getTargetTriple().isOSBinFormatCOFF()) {
      p_object_layer.setOverrideObjectFlagsWithResponsibilityFlags(true);
      p_object_layer.setAutoClaimResponsibilityForObjectSymbols(true);
    }
  }

  ~jit() {
    if (auto Err = p_ES->endSession())
      p_ES->reportError(std::move(Err));
  }

  static std::unique_ptr<jit> Create() {
    auto EPC = SelfExecutorProcessControl::Create();
    if (!EPC) {
      fprintf(stderr, toString(EPC.takeError()).c_str());
      return nullptr;
    }
    auto ES = std::make_unique<ExecutionSession>(std::move(*EPC));

    JITTargetMachineBuilder JTMB(
        ES->getExecutorProcessControl().getTargetTriple());

    auto DL = JTMB.getDefaultDataLayoutForTarget();
    if (!DL) {
      Error e = ES->endSession();
      fprintf(stderr, toString(DL.takeError()).c_str());
      return nullptr;
    }
    return std::make_unique<jit>(std::move(ES), JTMB,std::move(*DL));
  }

  DataLayout getDataLayout() {
    return p_data_layout;
  }

  JITDylib &getMainJITDylib() { return p_main_jd; }

  Error addModule(ThreadSafeModule TSM, ResourceTrackerSP RT = nullptr) {
    if (!RT)
      RT = p_main_jd.getDefaultResourceTracker();
    return p_compile_layer.add(RT, std::move(TSM));
  }

  Expected<ExecutorSymbolDef> lookup(StringRef Name) {
    return p_ES->lookup({&p_main_jd}, p_mangle(Name.str()));
  }
};

} // namespace turas