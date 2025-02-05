#include "ast.h"
#include "token.h"
#include "parse.h"
#include "generate.h"
#include "jit.h"
#include "llvm/Support/Error.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar/GVN.h"
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
      auto err = jit::s_jit->addModule(ThreadSafeModule(
          std::move(Generator::m_module), std::move(Generator::m_context)
          ));
      if(err)
      {
        fprintf(stderr, "Failed to add function definition to module");
      }
      Generator::init_generator();
      Generator::init_opt_passes();

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
      Generator::m_function_protos[ext->m_name] = std::move(ext);
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
      auto rt = jit::s_jit->getMainJITDylib().createResourceTracker();
      auto tsm = ThreadSafeModule(std::move(Generator::m_module),
                                  std::move(Generator::m_context));

      if(Error err = jit::s_jit->addModule(std::move(tsm), rt))
      {
        fprintf(stderr, "Failed to add module to JIT runtime");
      }

      fn_ir->print(errs());

      // module cannot be modified, so create a new one
      Generator::init_generator();
      Generator::init_opt_passes();

      auto ExprSymbol = jit::s_jit->lookup("__anon_expr");
      assert(ExprSymbol && "Function not found");

      double (*FP)() = ExprSymbol->getAddress().toPtr<double (*)()>();
      fprintf(stderr, "Evaluated to %f\n", FP());

      auto error = rt->remove();

      if(error)
      {
        fprintf(stderr, "Failed to add delete anonymous expression from JIT runtime");
      }
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


#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" DLLEXPORT double printd(double X) {
  fprintf(stderr, "%f\n", X);
  return 0;
}

/// putchard - putchar that takes a double prints it as a char "%f\n", returning 0.
extern "C" DLLEXPORT double putchard(double X) {
  putchar(static_cast<int>(X));
  return 0;
}

int main()
{
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  Tokenizer::init_tokenizer_presedence();

  fprintf(stderr, "ready> ");

  Tokenizer::get_next_token();

  jit::s_jit = std::move(jit::Create());

  Generator::init_generator();
  Generator::init_opt_passes();

  MainLoop();

  Generator::m_module->print(errs(), nullptr);
  return 0;
}