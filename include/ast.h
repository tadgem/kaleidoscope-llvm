
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


// Prototypes

class PrototypeAST
{
public:
  std::string m_name;
  std::vector<std::string> m_args;

  virtual ~PrototypeAST() = default;

  PrototypeAST(const std::string& name, std::vector<std::string> args) :
  m_name(name), m_args(std::move(args)) {};
  llvm::Function *codegen();

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