//
// Created by liam_ on 2/1/2025.
//
#include "ast.h"
#include "generate.h"
#include "parse.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Verifier.h>
using namespace llvm;

Value *kal::NumberExprAST::codegen() {
  return ConstantFP::get(*Generator::m_context, APFloat(m_value));
}

Value *kal::VariableExprAST::codegen() {
  Value* v = Generator::m_named_values[m_name];

  if(!v)
  {
    return Helpers::LogErrorValue("Unknown variable name");
  }

  return v;
}
llvm::Value *kal::BinaryExprAST::codegen() {
  Value* L = m_lhs->codegen();
  Value* R = m_rhs->codegen();

  if(!L || !R)
  {
    return nullptr;
  }

  switch(m_op)
  {
  case '+':
    return Generator::m_ir_builder->CreateFAdd(L, R, "addtmp");
  case '-':
    return Generator::m_ir_builder->CreateFSub(L, R, "subtmp");
  case '*':
    return Generator::m_ir_builder->CreateFMul(L, R, "multmp");
  case '<':
    L = Generator::m_ir_builder->CreateFCmpULT(L, R, "cmptmp");
    return Generator::m_ir_builder->CreateUIToFP(
        L, Type::getDoubleTy(*Generator::m_context));
  default:
    return Helpers::LogErrorValue("Invalid binary operator");
  }
}
llvm::Value *kal::CallExprAST::codegen() {
  Function* callee_func = Generator::m_module->getFunction(m_callee);
  if(!callee_func)
  {
    return Helpers::LogErrorValue("Unknown function referenced");
  }

  if(callee_func->arg_size() != m_args.size())
  {
    return Helpers::LogErrorValue("Incorrect number of arguments passed to function");
  }

  std::vector<Value*> arg_values;
  for(auto i = 0; i < callee_func->arg_size(); i++)
  {
    arg_values.push_back(m_args[i]->codegen());
    if(!arg_values.back())
    {
      return Helpers::LogErrorValue("Invalid argument provided");
    }
  }
  return Generator::m_ir_builder->CreateCall(callee_func, arg_values, "calltmp");
}

llvm::Function *kal::PrototypeAST::codegen() {
  std::vector<Type*> Doubles(m_args.size(),
                              Type::getDoubleTy(*Generator::m_context));

  FunctionType* ft = FunctionType::get(
      Type::getDoubleTy(*Generator::m_context), Doubles, false);

  Function* func = Function::Create(ft, Function::ExternalLinkage, m_name, Generator::m_module.get());

  unsigned int index = 0;
  for(auto& arg : func->args())
  {
    arg.setName(m_args[index++]);
  }
  return func;
}

llvm::Function *kal::FunctionAST::codegen() {
  Function* f = Generator::m_module->getFunction(m_proto->m_name);

  if(f) {
    if (f->arg_size() != m_proto->m_args.size()) {
      return (Function *)Helpers::LogErrorValue(
          "Mismatched argument count from previous function definition");
    }

    unsigned int index = 0;
    for (auto &arg : f->args()) {
      if (arg.getName() != m_proto->m_args[index++]) {
        return (Function *)Helpers::LogErrorValue(
            "Mismatched argument names in function");
      }
    }
  }

  // if function has not already been defined (not found in the module)
  if(!f)
  {
    f = m_proto->codegen();
  }

  // failed to generate a function proto
  if(!f)
  {
    return nullptr;
  }

  // Function has already been defined if not empty;
  if(!f->empty())
  {
    return (Function*)Helpers::LogErrorValue("Function cannot be redefined");
  }

  BasicBlock* bb = BasicBlock::Create(*Generator::m_context, "entry", f);
  Generator::m_ir_builder->SetInsertPoint(bb);

  Generator::m_named_values.clear();
  for(auto& arg : f->args())
  {
    Generator::m_named_values[std::string(arg.getName())] = &arg;
  }

  if(Value* ret_val = m_body->codegen())
  {
    Generator::m_ir_builder->CreateRet(ret_val);


    verifyFunction(*f);

    return f;
  }
  f->eraseFromParent();
  return nullptr;
}
