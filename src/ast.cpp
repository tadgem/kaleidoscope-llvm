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
  if(Generator::m_debug)
  {
    Generator::m_debug_info.emit_location(this);
  }
  return ConstantFP::get(*Generator::m_context, APFloat(m_value));
}

Value *kal::VariableExprAST::codegen() {
  AllocaInst* v = Generator::m_named_values[m_name];

  if(!v)
  {
    return Helpers::LogErrorValue("Unknown variable name");
  }
  if(Generator::m_debug)
  {
    Generator::m_debug_info.emit_location(this);
  }
  return Generator::m_ir_builder->CreateLoad(
      v->getAllocatedType(), v, m_name.c_str());
}
llvm::Value *kal::BinaryExprAST::codegen() {
  if(Generator::m_debug)
  {
    Generator::m_debug_info.emit_location(this);
  }
  // special case for assignment
  if(m_op == '=')
  {
    auto* lhse = static_cast<VariableExprAST*>(m_lhs.get());
    if(!lhse)
    {
      return Helpers::LogErrorValue("Destination of '=' must be a variable");
    }

    llvm::Value* val = m_rhs->codegen();
    if(!val)
    {
      return nullptr;
    }
    Value* var = Generator::m_named_values[std::string(lhse->m_name)];
    if(!var)
    {
      return Helpers::LogErrorValue("Unknown variable name");
    }

    Generator::m_ir_builder->CreateStore(val, var);
    return val;
  }
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
    break;
  }

  Function* f = Helpers::GetFunction(std::string("binary") + char(m_op));
  assert(f && "binary operator not found");

  Value* ops[2] = {L, R};
  return Generator::m_ir_builder->CreateCall(f,ops, "binop");
}
llvm::Value *kal::CallExprAST::codegen() {
  if(Generator::m_debug)
  {
    Generator::m_debug_info.emit_location(this);
  }
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

  auto& p = *m_proto;
  Generator::m_function_protos[m_proto->m_name] = std::move(m_proto);
  Function* f = Helpers::GetFunction(p.m_name);
  if(!f)
  {
    return nullptr;
  }
  DISubprogram *sub_prog;
  DIFile *unit;
  unsigned int line_no = p.m_line_no;
  if(Generator::m_debug) {
    unit = Generator::m_debug_builder->createFile(
        Generator::m_debug_info.m_compile_unit->getFilename(),
        Generator::m_debug_info.m_compile_unit->getDirectory());

    DIScope *func_context = unit;
    unsigned int scope_line = 0;

    sub_prog = Generator::m_debug_builder->createFunction(
        func_context, p.m_name, StringRef(), unit, line_no,
        Helpers::CreateDebugFunctionType(f->arg_size()), scope_line,
        DINode::FlagPrototyped, DISubprogram::SPFlagDefinition);
    f->setSubprogram(sub_prog);
    Generator::m_debug_info.m_lexical_blocks.push_back(sub_prog);
    Generator::m_debug_info.emit_location(nullptr);
  }
  if(p.m_is_operator)
  {
    Tokenizer::s_op_precedence[p.get_operator_name()] = p.m_precedence;
  }

  BasicBlock* bb = BasicBlock::Create(*Generator::m_context, "entry", f);
  Generator::m_ir_builder->SetInsertPoint(bb);

  Generator::m_named_values.clear();
  int arg_index = 0;
  for(auto& arg : f->args())
  {
    AllocaInst* _alloca = Helpers::CreateEntryBlockAlloca(f, std::string(arg.getName()));
    if(Generator::m_debug)
    {
      DILocalVariable* d = Generator::m_debug_builder->createParameterVariable(
      sub_prog, arg.getName(), ++arg_index, unit, line_no, Generator::m_debug_info.get_double_type());

      Generator::m_debug_builder->insertDeclare(_alloca, d,
        Generator::m_debug_builder->createExpression(),
        DILocation::get(sub_prog->getContext(), line_no, 0, sub_prog),
        Generator::m_ir_builder->GetInsertBlock());

    }
    Generator::m_ir_builder->CreateStore(&arg, _alloca);

    Generator::m_named_values[std::string(arg.getName())] = _alloca;
  }

  Generator::m_debug_info.emit_location(m_body.get());

  if(Value* ret_val = m_body->codegen())
  {
    Generator::m_ir_builder->CreateRet(ret_val);
    verifyFunction(*f);
    Generator::m_FPM->run(*f, *Generator::m_FAM);
    if(Generator::m_debug)
    {
      Generator::m_debug_info.m_lexical_blocks.pop_back();
    }
    return f;
  }

  f->eraseFromParent();
  if(p.is_binary_op())
  {
    Tokenizer::s_op_precedence.erase(p.get_operator_name());
  }
  return nullptr;
}
llvm::Value *kal::IfExprAST::codegen() {
  if(Generator::m_debug)
  {
    Generator::m_debug_info.emit_location(this);
  }
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
  Function* func = Generator::m_ir_builder->GetInsertBlock()->getParent();
  AllocaInst* _alloca = Helpers::CreateEntryBlockAlloca(func, m_variable_name);
  if(Generator::m_debug)
  {
    Generator::m_debug_info.emit_location(this);
  }
  Value* start_v = m_start->codegen();
  if(!start_v)
  {
    return nullptr;
  }

  Generator::m_ir_builder->CreateStore(start_v, _alloca);
  BasicBlock* preheader_bb = Generator::m_ir_builder->GetInsertBlock();
  BasicBlock* loop_bb = BasicBlock::Create(*Generator::m_context, "loop", func);

  Generator::m_ir_builder->CreateBr(loop_bb);
  Generator::m_ir_builder->SetInsertPoint(loop_bb);

  PHINode* var = Generator::m_ir_builder->CreatePHI(
      Type::getDoubleTy(*Generator::m_context),
      2,m_variable_name);

  var->addIncoming(start_v, preheader_bb);

  Value* old_value = Generator::m_named_values[m_variable_name];
  Generator::m_named_values[m_variable_name] = _alloca;

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

  Value* end_cond = m_end->codegen();
  if(!end_cond)
  {
    return nullptr;
  }

  Value* cur_var = Generator::m_ir_builder->CreateLoad(
      _alloca->getAllocatedType(), _alloca, m_variable_name.c_str());
  Value* next_var = Generator::m_ir_builder->CreateFAdd(
      cur_var, step_v, "nextvar");
  Generator::m_ir_builder->CreateStore(next_var, _alloca);

  end_cond = Generator::m_ir_builder->CreateFCmpONE(
      end_cond, ConstantFP::get(*Generator::m_context, APFloat(0.0)), "loopcond");

  BasicBlock* loop_end_bb = Generator::m_ir_builder->GetInsertBlock();
  BasicBlock* after_bb = BasicBlock::Create(*Generator::m_context, "afterloop", func);

  Generator::m_ir_builder->CreateCondBr(end_cond, loop_bb, after_bb);
  Generator::m_ir_builder->SetInsertPoint(after_bb);

  var->addIncoming(next_var, loop_end_bb);

  if(old_value)
  {
    Generator::m_named_values[m_variable_name] = _alloca;
  }
  else
  {
    Generator::m_named_values.erase(m_variable_name);
  }

  return ConstantFP::getNullValue(Type::getDoubleTy(*Generator::m_context));
}
llvm::Value *kal::UnaryExprAST::codegen() {
  if(Generator::m_debug)
  {
    Generator::m_debug_info.emit_location(this);
  }
  Value* op_v = m_operand->codegen();
  if(!op_v)
  {
    return nullptr;
  }
  Function* f = Helpers::GetFunction(std::string("unary") + m_op_code);
  if(!f)
  {
    return Helpers::LogErrorValue("Unknown unary operator");
  }
  return Generator::m_ir_builder->CreateCall(f, op_v, "unop");
}
llvm::Value *kal::VariableAssignmentExpr::codegen() {

  std::vector<AllocaInst*> old_bindings;

  llvm::Function* func = Generator::m_ir_builder->GetInsertBlock()->getParent();
  for(auto i = 0; i < m_var_names.size(); i++)
  {
    const std::string& name = m_var_names[i].first;
    ExprAST* init = m_var_names[i].second.get();
    Value* init_value;
    if(init)
    {
      init_value = init->codegen();
      if(!init_value)
      {
        return nullptr;
      }
    }
    else
    {
      init_value = ConstantFP::get(*Generator::m_context, APFloat(0.0));
    }

    AllocaInst* _alloca = Helpers::CreateEntryBlockAlloca(func, name);
    Generator::m_ir_builder->CreateStore(init_value, _alloca);

    old_bindings.push_back(Generator::m_named_values[name]);
    Generator::m_named_values[name] = _alloca;
  }
  if(Generator::m_debug)
  {
    Generator::m_debug_info.emit_location(this);
  }
  Value* body_val = m_body->codegen();
  if(!body_val)
  {
    return nullptr;
  }
  for(int i = 0; i < old_bindings.size(); i++)
  {
    Generator::m_named_values[m_var_names[i].first] = old_bindings[i];
  }
  return body_val;
}
