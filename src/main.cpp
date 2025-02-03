#include "ast.h"
#include "token.h"
#include "parse.h"
#include "generate.h"

using namespace kal;

void HandleDefinition()
{
  if(auto ast = Parser::ParseDefinition())
  {
    if(auto* fn_ir = ast->codegen())
    {
      fprintf(stderr, "Parsed a definition\n");
      fn_ir->print(errs());
      fprintf(stderr, "\n");
    }
  }
  else
  {
    Tokenizer::get_next_token();
  }
}


void HandleExtern()
{
  if(auto ext = Parser::ParseExtern())
  {
    if(auto* fn_ir = ext->codegen())
    {
      fprintf(stderr, "Parsed an external\n");
      fn_ir->print(errs());
      fprintf(stderr, "\n");
    }
  }
  else
  {
    Tokenizer::get_next_token();
  }
}

void HandleTopLevelExpressions()
{
  if(auto tle = Parser::ParseTopLevelExpr())
  {
    if(auto* fn_ir = tle->codegen())
    {
      fprintf(stderr, "Parsed a top level expression\n");
      fn_ir->print(errs());
      fprintf(stderr, "\n");
    }
  }
  else
  {
    Tokenizer::get_next_token();
  }
}

static void MainLoop()
{
  while(true)
  {
    fprintf(stderr, "ready> ");
    switch(Tokenizer::s_current_token)
    {
      case Token::END_OF_FILE:
        fprintf(stderr, "End of file encountered");
        return;
      case ';':
        Tokenizer::get_next_token();
        break;
      case Token::DEF:
        HandleDefinition();
        break;
      case Token::EXTERN:
        HandleExtern();
        break;
      default:
        HandleTopLevelExpressions();
        break;
    }

  }
}

int main()
{
  Tokenizer::init_tokenizer_presedence();
  Generator::init_generator();

  fprintf(stderr, "ready> ");
  Tokenizer::get_next_token();
  MainLoop();

  Generator::m_module->print(errs(), nullptr);
  return 0;
}