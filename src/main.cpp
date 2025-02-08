#include "ast.h"
#include "token.h"
#include "parse.h"
#include "generate.h"
#include "jit.h"
#include "target.h"
#include "llvm/Support/Error.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/IR/LegacyPassManager.h"

using namespace kal;

void HandleDefinition()
{
  if(auto ast = Parser::ParseDefinition())
  {
    if(auto* fn_ir = ast->codegen())
    {
      if(Generator::m_output_statement_ir_to_console) {
        fprintf(stderr, "Parsed a definition\n");
        fn_ir->print(errs());
        fprintf(stderr, "\n");
      }
      if(target::m_use_jit)
      {
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
      if(Generator::m_output_statement_ir_to_console) {
        fprintf(stderr, "Parsed an external\n");
        fn_ir->print(errs());
        fprintf(stderr, "\n");
      }
      Generator::m_function_protos[ext->m_name] = std::move(ext);
    }
    else
    {
      fprintf(stderr, "Error parsing external");
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
      if(target::m_use_jit) {
        auto rt = jit::s_jit->getMainJITDylib().createResourceTracker();
        auto tsm = ThreadSafeModule(std::move(Generator::m_module),
                                    std::move(Generator::m_context));

        if (Error err = jit::s_jit->addModule(std::move(tsm), rt)) {
          fprintf(stderr, "Failed to add module to JIT runtime");
        }
        if(Generator::m_output_statement_ir_to_console)
        {
          fn_ir->print(errs());
        }

        // module cannot be modified, so create a new one
        Generator::init_generator();
        Generator::init_opt_passes();

        auto ExprSymbol = jit::s_jit->lookup("main");
        assert(ExprSymbol && "Function not found");

        double (*FP)() = ExprSymbol->getAddress().toPtr<double (*)()>();
        fprintf(stderr, "Evaluated to %f\n", FP());

        auto error = rt->remove();

        if (error) {
          fprintf(stderr,
                  "Failed to add delete anonymous expression from JIT runtime");
        }
      }
    }
    else
    {
      fprintf(stderr, "Error generating top level expression");
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
    if(target::m_use_jit)
    {
      fprintf(stderr, "ready> ");
    }

    switch(Tokenizer::s_current_token)
    {
      case Token::END_OF_FILE:
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

int main() {
  Generator::m_debug = true;
  Generator::m_output_object_file = false;
  target::m_use_jit = false;

  target::init_target();
  Tokenizer::init_tokenizer_presedence();

  if (target::m_use_jit) {
    fprintf(stderr, "ready> ");
  }
  Tokenizer::get_next_token();
  jit::s_jit = std::move(jit::Create());
  Generator::init_generator();
  Generator::init_opt_passes();

  MainLoop();

  if(!target::m_use_jit && Generator::m_output_object_file) {
    auto filename = "output.o";
    std::error_code ec;
    llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);

    if (ec) {
        llvm::errs() << "could not open file for writing object file"
                     << ec.message() << "\n";
        return 1;
    }

    legacy::PassManager pass_manager;
    auto filetype = llvm::CodeGenFileType::ObjectFile;
    if (target::m_target_machine->addPassesToEmitFile(pass_manager, dest,
                                                      nullptr, filetype)) {
        llvm::errs() << "Target Machine cannot emit an object file\n";
        return 1;
    }

    pass_manager.run(*Generator::m_module);
    dest.flush();
  }
  else
  {
    if(Generator::m_debug)
    {
        Generator::m_debug_builder->finalize();
    }
    Generator::m_module->print(errs(), nullptr);
  }
  return 0;
}