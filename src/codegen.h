// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef INCLUDE_SRC_CODEGEN_H_
#define INCLUDE_SRC_CODEGEN_H_
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include "expr.h"
using llvm::IRBuilder;
using llvm::LLVMContext;
using llvm::Module;
using llvm::Value;

// Generate code with LLVM
class CodeGen : public Visitor<Value*> {
 private:
  llvm::LLVMContext* context;
  llvm::IRBuilder<>* builder;
  llvm::Module* module;

 public:
  // Constructor (no implementation)
  CodeGen(LLVMContext* context, IRBuilder<>* builder, Module* module)
      : context(context), builder(builder), module(module) {}
  // Generate code for a prefix expr
  Value* visitPrefixExpr(Expr* expr) override {
    Value* exp = _visit(expr->getPrefix()->expr.get());
    // Add int or float token
    if (expr->type == TOKEN_TYPE::INT) {
      return builder->CreateMul(
          exp, llvm::ConstantInt::get(*context, llvm::APInt(32, -1, true)));
    } else {
      return builder->CreateFMul(
          exp, llvm::ConstantFP::get(*context, llvm::APFloat(-1.0f)));
    }
  }
  // Generate code for an int expr
  Value* visitIntExpr(Expr* expr) override {
    return llvm::ConstantInt::get(*context,
                                  llvm::APInt(32, expr->getInt(), true));
  }
  // Generate code for a float expr
  Value* visitFloatExpr(Expr* expr) override {
    return llvm::ConstantFP::get(*context, llvm::APFloat(expr->getFloat()));
  }
  // Generate code for a binary expr, visiting both children 
  Value* visitBinaryExpr(Expr* expr) override {
    Value* left = _visit(expr->getBinary()->left.get());
    Value* right = _visit(expr->getBinary()->right.get());
    if (expr->type == TOKEN_TYPE::INT) {
      // Add int operations
      switch (expr->getBinary()->op) {
        case TOKEN_TYPE::PLUS:
          return builder->CreateAdd(left, right);
        case TOKEN_TYPE::MINUS:
          return builder->CreateSub(left, right);
        case TOKEN_TYPE::STAR:
          return builder->CreateMul(left, right);
        case TOKEN_TYPE::SLASH:
          return builder->CreateSDiv(left, right);
        default:
          return nullptr;
      }
    } else {
      // Add float operations
      switch (expr->getBinary()->op) {
        case TOKEN_TYPE::PLUS:
          return builder->CreateFAdd(left, right);
        case TOKEN_TYPE::MINUS:
          return builder->CreateFSub(left, right);
        case TOKEN_TYPE::STAR:
          return builder->CreateFMul(left, right);
        case TOKEN_TYPE::SLASH:
          return builder->CreateFDiv(left, right);
        default:
          return nullptr;
      }
    }
  }
  // Generate code for implicit type conversions
  Value* visitImplicitTypeConvExpr(Expr* expr) override {
    Value* exp = _visit(expr->getImplicitTypeConvExpr()->expr.get());
    return builder->CreateSIToFP(exp, builder->getDoubleTy());
  }
  // Enter a visitor (no implementation)
  void enterVisitor() override {}
  // Exit a visitor (no implementation)
  void exitVisitor() override {}
  // Use default copy and destructor operations
  CodeGen(CodeGen&&) = default;
  CodeGen(const CodeGen&) = default;
  CodeGen& operator=(CodeGen&&) = default;
  CodeGen& operator=(const CodeGen&) = default;

  ~CodeGen() = default;

 private:
};

#endif  // INCLUDE_SRC_CODEGEN_H_
