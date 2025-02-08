//
// Created by liam_ on 2/7/2025.
//

#pragma once
#include <memory>
#include <string>

#include "llvm/Target/TargetMachine.h"
namespace kal {
class target {
public:

  static void init_target();

  inline static llvm::TargetMachine*                  m_target_machine;
  inline static std::string                           m_target_triple;
  inline static bool                                  m_use_jit = true;
};
} // namespace turas