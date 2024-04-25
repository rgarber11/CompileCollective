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
// #include "PostFixExprVisualizer.h"
// #include "codegen.h"
#include <iostream>
#include <vector>

#include "lexer.h"
#include "token.h"
#include "parser.h"
#include "type_checker.h"
#include "codegen.h"
std::string readFile(const char* path) {
  std::ifstream file(path);
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}
 /*
void createMain(LLVMContext* context, Module* module, IRBuilder<>* builder,
                Value* val) {
  const auto func_type = llvm::FunctionType::get(builder->getInt32Ty(), false);
  const auto mainer = llvm::Function::Create(
      func_type, llvm::Function::ExternalLinkage, "main", module);
  const auto entry = llvm::BasicBlock::Create(*context, "entrypoint", mainer);
  builder->SetInsertPoint(entry);

  llvm::Constant* str;
  llvm::FunctionType* printf_type;
  if (val->getType() == builder->getInt32Ty()) {
    str = builder->CreateGlobalStringPtr("%d\n");
    const std::vector<llvm::Type*> printf_args = {
        builder->getInt8Ty()->getPointerTo(), builder->getInt32Ty()};
    const llvm::ArrayRef printf_args_ref(printf_args);
    printf_type =
        llvm::FunctionType::get(builder->getInt32Ty(), printf_args_ref, true);
  } else {
    str = builder->CreateGlobalStringPtr("%f\n");
    const std::vector<llvm::Type*> printf_args = {
        builder->getInt8Ty()->getPointerTo(), builder->getDoubleTy()};
    const llvm::ArrayRef printf_args_ref(printf_args);
    printf_type =
        llvm::FunctionType::get(builder->getInt32Ty(), printf_args_ref, true);
  }
  const auto printfer = module->getOrInsertFunction("printf", printf_type);
  builder->CreateCall(printfer, {str, val});
  builder->CreateRet(
      llvm::ConstantInt::get(builder->getInt32Ty(), llvm::APInt(32, 0, true)));
  module->dump();
}
  */
void writeModuleToFile(
    Module* module,
    const char*
        path)  // For now using answer from
               //
//https://stackoverflow.com/questions/11657529/how-to-generate-an-executable-from-an-llvmmodule
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
// Main method (entry point, used for testing)
int main(int argc, char* argv[]) {
  if (argc != 3) return 255;
  std::string input = readFile(argv[1]);
  Parser parser{Lexer{input}};
  auto env = parser.parse();
  TypeChecker type_checker{env.get()};
  type_checker.visit();

  llvm::LLVMContext
      context;  // Based off https://layle.me/posts/using-llvm-with-cmake/
  llvm::IRBuilder builder(context);
  const auto module = std::make_unique<llvm::Module>("first type", context);
  CodeGen code_gen(env.get(), &context, &builder, module.get());
  llvm::Value* val = code_gen.visitDeclarationStmt(env->getMember("main"));
  module->dump();
  writeModuleToFile(module.get(), argv[2]);
  return 0;
}
