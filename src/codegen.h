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

class CodeGen : public Visitor<Value*> {
 private:
  llvm::LLVMContext* context;
  llvm::IRBuilder<>* builder;
  llvm::Module* module;

 public:
  CodeGen(LLVMContext* context, IRBuilder<>* builder, Module* module)
      : context(context), builder(builder), module(module){};
  Value* visitPrefixExpr(Expr* expr) override {
    Value* exp = _visit(expr->getPrefix()->expr.get());
    if(exp->getType() == builder->getInt32Ty()) {
      return builder->CreateMul(
          exp, llvm::ConstantInt::get(*context, llvm::APInt(32, -1, true)));
    } else {
      return builder->CreateFMul(
          exp, llvm::ConstantFP::get(*context, llvm::APFloat((double)-1.0f)));
    }
  }
  Value* visitIntExpr(Expr* expr) override {
    return llvm::ConstantInt::get(*context,
                                  llvm::APInt(32, expr->getInt(), true));
  }
  Value* visitFloatExpr(Expr* expr) override {
    return llvm::ConstantFP::get(*context, llvm::APFloat(expr->getFloat()));
  }
  Value* visitBinaryExpr(Expr* expr) override {
    Value* left = _visit(expr->getBinary()->left.get());
    Value* right = _visit(expr->getBinary()->right.get());
    if(left->getType() == builder->getInt32Ty() && right->getType() == builder->getInt32Ty()) {
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
      if(left->getType() == builder->getInt32Ty()) {
        left = builder->CreateSIToFP(left, builder->getDoubleTy());
      }
      if(right->getType() == builder->getInt32Ty()) {
        right = builder->CreateSIToFP(right, builder->getDoubleTy());
      }
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
  void enterVisitor() override {}
  void exitVisitor() override {}
  CodeGen(CodeGen&&) = default;
  CodeGen(const CodeGen&) = default;
  CodeGen& operator=(CodeGen&&) = default;
  CodeGen& operator=(const CodeGen&) = default;

  ~CodeGen() = default;

 private:
};

#endif  // INCLUDE_SRC_CODEGEN_H_
