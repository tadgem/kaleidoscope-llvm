#include "parse.h"
std::unique_ptr<kal::ExprAST> kal::Parser::ParseExpression() {
  auto lhs = ParseUnaryExpr();

  if(!lhs)
  {
    return nullptr;
  }

  return ParseBinOpRHS(0, std::move(lhs));
}

std::unique_ptr<kal::ExprAST> kal::Parser::ParseParenExpr()
{
  Tokenizer::get_next_token();
  auto v = ParseExpression();
  if(!v) {
    return nullptr;
  }

  if(Tokenizer::s_current_token != ')')
  {
  return Helpers::LogErrorExpr("Expected a ')'");
  }

  Tokenizer::get_next_token();
  return v;
}

std::unique_ptr<kal::ExprAST> kal::Parser::ParseNumberExpression()
{
  auto result = std::make_unique<NumberExprAST>(Tokenizer::s_number_value);
  Tokenizer::get_next_token();
  return std::move(result);
}
std::unique_ptr<kal::ExprAST> kal::Parser::ParseIdentifierExpr() {
  std::string id_name = Tokenizer::s_identifier_str;

  Tokenizer::get_next_token();

  if(Tokenizer::s_current_token != '(')
  {
    return std::make_unique<VariableExprAST>(id_name);
  }

  Tokenizer::get_next_token();
  std::vector<std::unique_ptr<ExprAST>> args;

  if(Tokenizer::s_current_token != ')')
  {
    while(true)
    {
      if(auto arg = ParseExpression())
      {
        args.push_back(std::move(arg));
      }
      else
      {
        return nullptr;
      }

      if(Tokenizer::s_current_token == ')')
      {
        break;
      }

      if(Tokenizer::s_current_token != ',')
      {
        return Helpers::LogErrorExpr("Expeted a ')' or ',' in an argument list");
      }

      Tokenizer::get_next_token();
    }
  }
  Tokenizer::get_next_token();

  return std::make_unique<CallExprAST>(id_name, std::move(args));
}

std::unique_ptr<kal::ExprAST> kal::Parser::ParsePrimary() {
  switch(Tokenizer::s_current_token)
  {
    default:
    return Helpers::LogErrorExpr("Failed to identify expression type");
    case Token::IDENTIFIER:
    return ParseIdentifierExpr();
    case Token::NUMBER:
    return ParseNumberExpression();
    case '(':
    return ParseParenExpr();
    case Token::IF:
    return ParseIfExpr();
    case Token::FOR:
    return ParseForExpr();
  }
}
std::unique_ptr<kal::ExprAST>
kal::Parser::ParseBinOpRHS(int precedence, std::unique_ptr<ExprAST> lhs) {
  while(true)
  {
    // rhs is implicitly the current token
    int token_precedence = Tokenizer::get_token_precedence();
    // if LHS has higher precedence, just return that
    // as we want to process the LHS first and will naturally
    // move on to RHS as a lower precedence op
    if(token_precedence < precedence)
    {
      return lhs;
    }

    // current token is the binary op
    // lhs is current - 1
    // rhs is current + 1
    int binary_op = Tokenizer::s_current_token;
    // get rhs token
    Tokenizer::get_next_token();
    // get expression
    auto rhs = ParseUnaryExpr();
    if(!rhs)
    {
      Helpers::LogErrorExpr("Failed to parse expression : {}");
      return nullptr;
    }
    int next_precedence = Tokenizer::get_token_precedence();
    if (token_precedence < next_precedence) {
      rhs = ParseBinOpRHS(token_precedence +1, std::move(rhs));
      if(!rhs) {
        return nullptr;
      }
    }
    lhs = std::make_unique<BinaryExprAST>(binary_op,
                                          std::move(lhs), std::move(rhs));


  }
}
std::unique_ptr<kal::PrototypeAST> kal::Parser::ParsePrototype() {
  using namespace kal;

  std::string fn_name;

  int kind = 0;
  int binary_precedence = 0;

  switch(Tokenizer::s_current_token)
  {
  default:
    Helpers::LogErrorProrotype("Expected function name in prototype");
  case Token::IDENTIFIER:
    fn_name = Tokenizer::s_identifier_str;
    kind = 0;
    Tokenizer::get_next_token();
    break;
  case Token::UNARY:
    Tokenizer::get_next_token();
    if(!isascii(Tokenizer::s_current_token))
    {
      return Helpers::LogErrorProrotype("Expected a unary operator");
    }
    fn_name = "unary";
    fn_name += (char) Tokenizer::s_current_token;
    kind = 1;
    Tokenizer::get_next_token();
    break;
  case Token::BINARY:
    Tokenizer::get_next_token();
    if(!isascii(Tokenizer::s_current_token))
    {
      return Helpers::LogErrorProrotype("Expected a binary operator");
    }
    fn_name = "binary";
    fn_name += (char) Tokenizer::s_current_token;
    kind = 2;

    Tokenizer::get_next_token();
    if(Tokenizer::s_current_token == Token::NUMBER)
    {
      if(Tokenizer::s_number_value < 1 || Tokenizer::s_number_value > 100)
      {
        return Helpers::LogErrorProrotype("Invalid precdence, must be in range 1->100");
      }
      binary_precedence = static_cast<int>(Tokenizer::s_number_value);
      Tokenizer::get_next_token();
    }
    break;
  }

  if(Tokenizer::s_current_token != '(')
  {
    return Helpers::LogErrorProrotype("Expected '(' in prototype");
  }
  std::vector<std::string> arg_names;
  while(Tokenizer::get_next_token() == Token::IDENTIFIER)
  {
    arg_names.push_back(Tokenizer::s_identifier_str);
  }
  if(Tokenizer::s_current_token != ')')
  {
    return Helpers::LogErrorProrotype("Expected a ')' in prototype");
  }
  Tokenizer::get_next_token();

  if(kind && arg_names.size() != kind)
  {
    return Helpers::LogErrorProrotype("Invalid number of operands for operator");
  }

  return std::make_unique<PrototypeAST>(fn_name, arg_names, kind != 0, binary_precedence);
}
std::unique_ptr<kal::FunctionAST> kal::Parser::ParseDefinition() {
  Tokenizer::get_next_token();
  auto proto = ParsePrototype();

  if(!proto) {
    return nullptr;
  }

  if(auto E = ParseExpression())
  {
    return std::make_unique<FunctionAST>(std::move(proto), std::move(E));
  }

  return nullptr;
}
std::unique_ptr<kal::PrototypeAST> kal::Parser::ParseExtern() {
  Tokenizer::get_next_token();
  return ParsePrototype();
}
std::unique_ptr<kal::FunctionAST> kal::Parser::ParseTopLevelExpr() {
  if(auto E = ParseExpression())
  {
    auto proto = std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(proto), std::move(E));
  }
  return nullptr;
}
std::unique_ptr<kal::ExprAST> kal::Parser::ParseIfExpr() {

  // eat if token
  Tokenizer::get_next_token();

  auto condition_expr = ParseExpression();
  if(!condition_expr)
  {
    return nullptr;
  }

  if(Tokenizer::s_current_token != Token::THEN)
  {
    return Helpers::LogErrorExpr("Expected then in if statement");
  }
  // eat the then token
  Tokenizer::get_next_token();
  auto then_expr = ParseExpression();
  if(!then_expr)
  {
    return nullptr;
  }

  if(Tokenizer::s_current_token != Token::ELSE)
  {
    return Helpers::LogErrorExpr("Expected an else in if statement");
  }
  // eat else token
  Tokenizer::get_next_token();
  auto else_expr = ParseExpression();
  if(!else_expr)
  {
    return nullptr;
  }
  return std::make_unique<IfExprAST>(
      std::move(condition_expr),
      std::move(then_expr),
      std::move(else_expr)
      );
}
std::unique_ptr<kal::ExprAST> kal::Parser::ParseForExpr() {
  Tokenizer::get_next_token();
  if(Tokenizer::s_current_token != Token::IDENTIFIER)
  {
    return Helpers::LogErrorExpr("Expected identifier in for loop");
  }

  std::string var_name = Tokenizer::s_identifier_str;
  Tokenizer::get_next_token();

  if(Tokenizer::s_current_token != '=')
  {
    return Helpers::LogErrorExpr("expected a '=' after for loop identifier");
  }

  Tokenizer::get_next_token();

  auto start = ParseExpression();

  if(!start)
  {
    return nullptr;
  }
  if(Tokenizer::s_current_token != ',')
  {
    return Helpers::LogErrorExpr("Expected ',' after for start value");
  }

  Tokenizer::get_next_token();

  auto end = ParseExpression();
  if(!end)
  {
    return nullptr;
  }

  std::unique_ptr<ExprAST> step;
  if(Tokenizer::s_current_token == ',')
  {
    Tokenizer::get_next_token();
    step = ParseExpression();
    if(!step)
    {
      return nullptr;
    }
  }

  if(Tokenizer::s_current_token != Token::IN)
  {
    return Helpers::LogErrorExpr("Expected 'in' after for loop declaration");
  }

  Tokenizer::get_next_token();

  auto body = ParseExpression();
  if(!body)
  {
    return nullptr;
  }

  return std::make_unique<ForExprAST>(
      var_name,
      std::move(start),
      std::move(end),
      std::move(step),
      std::move(body));
}
std::unique_ptr<kal::ExprAST> kal::Parser::ParseUnaryExpr() {
  auto current_token = Tokenizer::s_current_token;
  if(!isascii(current_token) || current_token == '(' || current_token == ')')
  {
    return ParsePrimary();
  }

  int op = Tokenizer::s_current_token;
  Tokenizer::get_next_token();

  if(auto operand = ParseUnaryExpr())
  {
    return std::make_unique<UnaryExprAST>(op, std::move(operand));
  }
  return nullptr;
}
