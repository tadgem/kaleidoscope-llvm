#include "defines.h"

#pragma once
namespace kal {

// Expressions

class ExprAST {
public:
  virtual ~ExprAST() = default;
};

class NumberExprAST : public ExprAST
{
public:
  double m_value;
  NumberExprAST(double val) : m_value(val) {};
};

class VariableExprAST : public ExprAST
{
public:
  String m_name;
  VariableExprAST(const String& val) : m_name(val) {};
};

class BinaryExprAST : public ExprAST
{
public:
  char m_op;
  UPtr<ExprAST> m_lhs, m_rhs;

  BinaryExprAST(char op, UPtr<ExprAST> lhs,
                UPtr<ExprAST> rhs) : m_op(op),
                                                m_lhs(std::move(lhs)),
                                                m_rhs(std::move(rhs)) {};
};

class CallExprAST : public ExprAST
{
public:
  String m_callee;
  Vector<UPtr<ExprAST>> m_args;

  CallExprAST(const String& callee,
              Vector<UPtr<ExprAST>> args) :
              m_callee(callee), m_args(std::move(args)) {}
};


// Prototypes

class PrototypeAST
{
public:
  String m_name;
  Vector<String> m_args;

  virtual ~PrototypeAST() = default;

  PrototypeAST(const String& name, Vector<String> args) :
  m_name(name), m_args(std::move(args)) {};
};

class FunctionAST
{
public:
  UPtr<PrototypeAST> m_proto;
  UPtr<ExprAST>      m_body;

  FunctionAST(UPtr<PrototypeAST> proto, UPtr<ExprAST> body) :
  m_proto(std::move(proto)), m_body(std::move(body)) {}
};
}