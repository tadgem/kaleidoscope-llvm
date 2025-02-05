#pragma once
#include "helpers.h"
#include "token.h"

namespace kal {
class Parser {
public:

  static std::unique_ptr<ExprAST>       ParseExpression();

  static std::unique_ptr<ExprAST>       ParseNumberExpression();

  static std::unique_ptr<ExprAST>       ParseParenExpr();

  static std::unique_ptr<ExprAST>       ParseIdentifierExpr();

  static std::unique_ptr<ExprAST>       ParseBinOpRHS(int precedence,
                                                std::unique_ptr<ExprAST> lhs);

  static std::unique_ptr<ExprAST>       ParseIfExpr();

  static std::unique_ptr<ExprAST>       ParseForExpr();

  static std::unique_ptr<ExprAST>       ParseUnaryExpr();

  static std::unique_ptr<ExprAST>       ParsePrimary();

  static std::unique_ptr<PrototypeAST>  ParsePrototype();

  static std::unique_ptr<FunctionAST>   ParseDefinition();

  static std::unique_ptr<PrototypeAST>  ParseExtern();

  static std::unique_ptr<FunctionAST>   ParseTopLevelExpr();

};
} // namespace kal