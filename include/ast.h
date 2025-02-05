
#pragma once
#include <memory>
#include <vector>
#include <string>
#include "llvm/IR/Value.h"

namespace kal {

// Expressions

class ExprAST {
public:
  virtual ~ExprAST() = default;
  virtual llvm::Value *codegen() = 0;
};

class NumberExprAST : public ExprAST
{
public:
  double m_value;
  NumberExprAST(double val) : m_value(val) {};
  llvm::Value *codegen() override;

};

class VariableExprAST : public ExprAST
{
public:
  std::string m_name;
  VariableExprAST(const std::string& val) : m_name(val) {};
  llvm::Value *codegen() override;

};

class BinaryExprAST : public ExprAST
{
public:
  char m_op;
  std::unique_ptr<ExprAST> m_lhs, m_rhs;

  BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs,
                std::unique_ptr<ExprAST> rhs) : m_op(op),
                                                m_lhs(std::move(lhs)),
                                                m_rhs(std::move(rhs)) {};
  llvm::Value *codegen() override;

};

class CallExprAST : public ExprAST
{
public:
  std::string m_callee;
  std::vector<std::unique_ptr<ExprAST>> m_args;

  CallExprAST(const std::string& callee,
              std::vector<std::unique_ptr<ExprAST>> args) :
              m_callee(callee), m_args(std::move(args)) {}
  llvm::Value *codegen() override;

};

class IfExprAST : public ExprAST
{
public:
  std::unique_ptr<ExprAST> m_cond, m_then, m_else;

  IfExprAST(std::unique_ptr<ExprAST> cond,
            std::unique_ptr<ExprAST> then,
            std::unique_ptr<ExprAST> _else) :
  m_cond(std::move(cond)), m_then(std::move(then)), m_else(std::move(_else))
  {

  }
  llvm::Value *codegen() override;
};

class ForExprAST : public ExprAST
{
public:
  std::string m_variable_name;
  std::unique_ptr<ExprAST> m_start, m_end, m_step, m_body;

  ForExprAST(const std::string var_name,
             std::unique_ptr<ExprAST> start,
             std::unique_ptr<ExprAST> end,
             std::unique_ptr<ExprAST> step,
             std::unique_ptr<ExprAST> body)
      : m_variable_name(var_name),
        m_start(std::move(start)),
        m_end(std::move(end)),
        m_step(std::move(step)),
        m_body(std::move(body)) {};


  llvm::Value *codegen() override;
};


// Prototypes

class PrototypeAST
{
public:
  std::string m_name;
  std::vector<std::string> m_args;
  bool  m_is_operator;
  int   m_precedence;
  virtual ~PrototypeAST() = default;

  PrototypeAST(const std::string& name, std::vector<std::string> args,
               bool isOperator = false, int precedence = -1) :
  m_name(name), m_args(std::move(args)),
  m_is_operator(isOperator), m_precedence(precedence) {};

  llvm::Function *codegen();

  bool is_unary_op() const {return m_is_operator && m_args.size() == 1; }
  bool is_binary_op() const {return m_is_operator && m_args.size() == 2; }

  char get_operator_name() const
  {
    assert(m_is_operator);
    return m_name[m_name.size() - 1];
  }

};

class FunctionAST
{
public:
  std::unique_ptr<PrototypeAST> m_proto;
  std::unique_ptr<ExprAST>      m_body;

  FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body) :
  m_proto(std::move(proto)), m_body(std::move(body)) {}
  llvm::Function *codegen();

};
}