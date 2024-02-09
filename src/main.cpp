// Copyright (c) 2024 Compile Collective. All Rights Reserved.

#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

#include <fstream>
#include <sstream>

#include "PostFixExprVisualizer.h"
#include "codegen.h"
#include "int_optimizer.h"
#include "lexer.h"
#include "parser.h"
std::string readFile(const char* path) {
  std::ifstream file(path);
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}
void writeModuleToFile(
    Module* module,
    const char*
        path)  // For now using answer from
               // https://stackoverflow.com/questions/11657529/how-to-generate-an-executable-from-an-llvmmodule
{
  auto TargetTriple = llvm::sys::getDefaultTargetTriple();
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  std::string Error;
  auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
  auto CPU = "generic";
  auto Features = "";

  llvm::TargetOptions opt;
  auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features,
                                                   opt, llvm::Reloc::PIC_);

  module->setDataLayout(TargetMachine->createDataLayout());
  module->setTargetTriple(TargetTriple);
  llvm::legacy::PassManager pass;
  std::error_code EC;
  auto FileType = llvm::CGFT_ObjectFile;
  llvm::raw_fd_ostream dest(path, EC, llvm::sys::fs::OF_None);
  TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType);
  pass.run(*module);
}

int main(int argc, char* argv[]) {
  if (argc != 3) return 255;
  std::string input = readFile(argv[1]);
  Parser parser{Lexer{input}};
  auto expr = parser.parse();
  PostFixExprVisualizer printer{};
  IntOptimizer optimizer{};
  optimizer.visit(expr.get());
  printer.visit(expr.get());

  llvm::LLVMContext
      context;  // Based off https://layle.me/posts/using-llvm-with-cmake/
  llvm::IRBuilder builder(context);
  const auto module = std::make_unique<llvm::Module>("first type", context);
  const auto func_type = llvm::FunctionType::get(builder.getVoidTy(), false);
  const auto mainer = llvm::Function::Create(
      func_type, llvm::Function::ExternalLinkage, "main", module.get());
  const auto entry = llvm::BasicBlock::Create(context, "entrypoint", mainer);
  builder.SetInsertPoint(entry);
  const std::vector<llvm::Type*> printf_args = {
      builder.getInt8Ty()->getPointerTo(), builder.getInt32Ty()};
  const llvm::ArrayRef printf_args_ref(printf_args);
  const auto printf_type =
      llvm::FunctionType::get(builder.getInt32Ty(), printf_args_ref, false);
  const auto printfer = module->getOrInsertFunction("printf", printf_type);

  auto str = builder.CreateGlobalStringPtr("%d\n");
  CodeGen code_gen(&context, &builder, module.get());

  builder.CreateCall(printfer, {str, code_gen.visit(expr.get())});
  builder.CreateRetVoid();
  module->dump();
  writeModuleToFile(module.get(), argv[2]);

  return 0;
}
