#include "ast.h"
#include "token.h"
#include "parse.h"

using namespace kal;

void HandleDefinition()
{
  if(Parser::ParseDefinition())
  {
    fprintf(stderr, "Parsed a function definition\n");
  }
  else
  {
    Tokenizer::get_next_token();
  }
}


void HandleExtern()
{
  if(Parser::ParseExtern())
  {
    fprintf(stderr, "Parsed an external\n");
  }
  else
  {
    Tokenizer::get_next_token();
  }
}

void HandleTopLevelExpressions()
{
  if(Parser::ParseTopLevelExpr())
  {
    fprintf(stderr, "Parsed a top level expression\n");
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
  fprintf(stderr, "ready> ");
  Tokenizer::get_next_token();
  MainLoop();
  return 0;
}