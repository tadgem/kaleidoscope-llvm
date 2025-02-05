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
  Function* callee_func = Helpers::GetFunction(m_callee);
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
  Function* f = Helpers::GetFunction(m_proto->m_name);

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

    Generator::m_FPM->run(*f, *Generator::m_FAM);

    return f;
  }
  f->eraseFromParent();
  return nullptr;
}
llvm::Value *kal::IfExprAST::codegen() {

  auto* condition_v = m_cond->codegen();

  if(!condition_v)
  {
    return nullptr;
  }

  condition_v = Generator::m_ir_builder->CreateFCmpONE(
      condition_v, ConstantFP::get(*Generator::m_context, APFloat(0.0)));

  Function* func = Generator::m_ir_builder->GetInsertBlock()->getParent();
  // create then block and insert into function;
  BasicBlock* then_block = BasicBlock::Create(
      *Generator::m_context, "then", func);
  // create else/merge block but do _NOT_ insert in to function yet
  BasicBlock* else_block = BasicBlock::Create(*Generator::m_context, "else");
  BasicBlock* merge_block = BasicBlock::Create(*Generator::m_context, "ifcont");

  // set up then block
  Generator::m_ir_builder->CreateCondBr(condition_v, then_block, else_block);
  Generator::m_ir_builder->SetInsertPoint(then_block);
  auto* then_v = m_then->codegen();
  if(!then_v)
  {
    return nullptr;
  }
  // once finished then block, return to merge (statement after end of else)
  Generator::m_ir_builder->CreateBr(merge_block);

  // set up else block
  then_block = Generator::m_ir_builder->GetInsertBlock();
  func->insert(func->end(), else_block);
  Generator::m_ir_builder->SetInsertPoint(else_block);

  auto* else_v = m_else->codegen();
  if(!else_v)
  {
    return nullptr;
  }
  // once finished else, go back to merge block (statement after end of else)
  Generator::m_ir_builder->CreateBr(merge_block);
  else_block = Generator::m_ir_builder->GetInsertBlock();

  func->insert(func->end(), merge_block);
  Generator::m_ir_builder->SetInsertPoint(merge_block);

  PHINode* phi_node = Generator::m_ir_builder->CreatePHI(
      Type::getDoubleTy(*Generator::m_context),
      2,
      "iftmp"
      );

  phi_node->addIncoming(then_v, then_block);
  phi_node->addIncoming(else_v, else_block);

  return phi_node;
}
llvm::Value *kal::ForExprAST::codegen()
{
  Value* start_v = m_start->codegen();
  if(!start_v)
  {
    return nullptr;
  }
  Function* func = Generator::m_ir_builder->GetInsertBlock()->getParent();
  BasicBlock* preheader_bb = Generator::m_ir_builder->GetInsertBlock();
  BasicBlock* loop_bb = BasicBlock::Create(*Generator::m_context, "loop", func);

  Generator::m_ir_builder->CreateBr(loop_bb);
  Generator::m_ir_builder->SetInsertPoint(loop_bb);

  PHINode* var = Generator::m_ir_builder->CreatePHI(
      Type::getDoubleTy(*Generator::m_context),
      2,m_variable_name);

  var->addIncoming(start_v, preheader_bb);

  Value* old_value = Generator::m_named_values[m_variable_name];
  Generator::m_named_values[m_variable_name] = var;

  if(!m_body->codegen())
  {
    return nullptr;
  }

  Value* step_v = nullptr;
  if(m_step)
  {
    step_v = m_step->codegen();
    if(!step_v)
    {
      return nullptr;
    }
  }
  else
  {
    step_v = ConstantFP::get(*Generator::m_context, APFloat(1.0));
  }

  Value* next_var = Generator::m_ir_builder->CreateFAdd(var, step_v, "nextvar");

  Value* end_cond = m_end->codegen();
  if(!end_cond)
  {
    return nullptr;
  }

  end_cond = Generator::m_ir_builder->CreateFCmpONE(
      end_cond, ConstantFP::get(*Generator::m_context, APFloat(0.0)), "loopcond");

  BasicBlock* loop_end_bb = Generator::m_ir_builder->GetInsertBlock();
  BasicBlock* after_bb = BasicBlock::Create(*Generator::m_context, "afterloop", func);

  Generator::m_ir_builder->CreateCondBr(end_cond, loop_bb, after_bb);
  Generator::m_ir_builder->SetInsertPoint(after_bb);

  var->addIncoming(next_var, loop_end_bb);

  if(old_value)
  {
    Generator::m_named_values[m_variable_name] = old_value;
  }
  else
  {
    Generator::m_named_values.erase(m_variable_name);
  }

  return ConstantFP::getNullValue(Type::getDoubleTy(*Generator::m_context));
}
