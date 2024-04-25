// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef INCLUDE_SRC_CODEGEN_H_
#define INCLUDE_SRC_CODEGEN_H_
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include <unordered_map>

#include "environment.h"
#include "expr.h"
#include "stmt.h"
#include "token.h"
using llvm::IRBuilder;
using llvm::LLVMContext;
using llvm::Module;
using llvm::Value;

// Generate code with LLVM
class CodeGen : public ExprVisitor<Value*>, StmtVisitor<Value*> {
 private:
  llvm::LLVMContext* context;
  llvm::IRBuilder<>* builder;
  llvm::Module* module;
  Environment* program;
  std::unordered_map<std::string, llvm::AllocaInst*> varEnv;

 public:
  // Constructor (no implementation)
  CodeGen(Environment* program, LLVMContext* context, IRBuilder<>* builder,
          Module* module)
      : context(context), builder(builder), module(module), program(program) {
   llvm::Function::Create(llvm::FunctionType::get(builder->getInt8PtrTy(), true), llvm::GlobalValue::ExternalLinkage, "printf", module);
  }
  // Generate code for a prefix expr
  Value* visitPrefixExpr(Expr* expr) override {
    Value* exp = _visitExpr(expr->getPrefixExpr()->expr.get());
    // Add int or float token
    switch (expr->getPrefixExpr()->op) {
      case TOKEN_TYPE::MINUS:
        if (expr->type == program->bottomTypes.intType) {
          return builder->CreateMul(
              exp, llvm::ConstantInt::get(*context, llvm::APInt(32, -1, true)));
        } else {
          return builder->CreateFMul(
              exp, llvm::ConstantFP::get(*context, llvm::APFloat(-1.0f)));
        }
        break;
      case TOKEN_TYPE::NOT:
        if (expr->type == program->bottomTypes.intType) {
          return builder->CreateXor(exp, llvm::APInt(32, 2147483647, true));
        } else {
          return builder->CreateNot(exp);
        }
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
  Value* visitBoolExpr(Expr* expr) override {
    return llvm::ConstantInt::get(
        *context, llvm::APInt(1, expr->getBoolExpr()->val, true));
  }
  Value* visitCharExpr(Expr* expr) override {
    return llvm::ConstantInt::get(*context,
                                  llvm::APInt(8, expr->getCharExpr()->c, true));
  }
  Value* visitStringExpr(Expr* expr) override {
    return builder->CreateGlobalStringPtr(expr->getStringExpr()->str.c_str());
  }
  // Generate code for a binary expr, visiting both children
  Value* visitBinaryExpr(Expr* expr) override {
    Value* left = _visitExpr(expr->getBinaryExpr()->left.get());
    Value* right = _visitExpr(expr->getBinaryExpr()->right.get());
    if(expr->getBinaryExpr()->op == TOKEN_TYPE::ASSIGN) {
      return builder->CreateStore(right, varEnv[expr->getBinaryExpr()->left->getLiteralExpr()->name]);
    }
    if (expr->type == program->bottomTypes.intType) {
      // Add int operations
      switch (expr->getBinaryExpr()->op) {
        case TOKEN_TYPE::PLUS:
          return builder->CreateAdd(left, right);
        case TOKEN_TYPE::MINUS:
          return builder->CreateSub(left, right);
        case TOKEN_TYPE::STAR:
          return builder->CreateMul(left, right);
        case TOKEN_TYPE::SLASH:
          return builder->CreateSDiv(left, right);
        case TOKEN_TYPE::LSHIFT:
          return builder->CreateShl(left, right);
        case TOKEN_TYPE::RSHIFT:
          return builder->CreateAShr(left, right);
        case TOKEN_TYPE::LANGLE:
          return builder->CreateICmpSLT(left, right);
        case TOKEN_TYPE::RANGLE:
          return builder->CreateICmpSGE(left, right);
        case TOKEN_TYPE::BITAND:
          return builder->CreateAnd(left, right);
        case TOKEN_TYPE::BITOR:
          return builder->CreateOr(left, right);
        case TOKEN_TYPE::XOR:
          return builder->CreateXor(left, right);
        case TOKEN_TYPE::MOD:
          return builder->CreateSRem(left, right);
        case TOKEN_TYPE::EQUALS:
          return builder->CreateICmpSGE(left, right);
        case TOKEN_TYPE::NEQUALS:
          return builder->CreateICmpNE(left, right);
        default:
          return llvm::Constant::getNullValue(builder->getInt32Ty());
      }
    } else if (expr->type == program->bottomTypes.floatType) {
      // Add float operations
      switch (expr->getBinaryExpr()->op) {
        case TOKEN_TYPE::PLUS:
          return builder->CreateFAdd(left, right);
        case TOKEN_TYPE::MINUS:
          return builder->CreateFSub(left, right);
        case TOKEN_TYPE::STAR:
          return builder->CreateFMul(left, right);
        case TOKEN_TYPE::SLASH:
          return builder->CreateFDiv(left, right);
        case TOKEN_TYPE::LANGLE:
          return builder->CreateFCmpULT(left, right);
        case TOKEN_TYPE::RANGLE:
          return builder->CreateFCmpUGT(left, right);
        case TOKEN_TYPE::GEQ:
          return builder->CreateFCmpUGE(left, right);
        case TOKEN_TYPE::LEQ:
          return builder->CreateFCmpULE(left, right);
        case TOKEN_TYPE::EQUALS:
          return builder->CreateFCmpUEQ(left, right);
        case TOKEN_TYPE::NEQUALS:
          return builder->CreateFCmpUNE(left, right);
        default:
          return llvm::Constant::getNullValue(builder->getInt32Ty());
      }
    } else if (expr->type == program->bottomTypes.boolType) {
      if(expr->getBinaryExpr()->left->type == program->bottomTypes.intType) {
        switch (expr->getBinaryExpr()->op) {
          case TOKEN_TYPE::OR:
            return builder->CreateOr(left, right);
          case TOKEN_TYPE::AND:
            return builder->CreateAnd(left, right);
          case TOKEN_TYPE::LANGLE:
            return builder->CreateICmpSLT(left, right);
          case TOKEN_TYPE::RANGLE:
            return builder->CreateICmpSGE(left, right);
        }
      } else if(expr->getBinaryExpr()->left->type == program->bottomTypes.boolType){
        switch (expr->getBinaryExpr()->op) {
          case TOKEN_TYPE::OR:
            return builder->CreateOr(left, right);
          case TOKEN_TYPE::AND:
            return builder->CreateAnd(left, right);
      }
    } else {
      switch (expr->getBinaryExpr()->op) {
        case TOKEN_TYPE::LANGLE:
          return builder->CreateFCmpULT(left, right);
        case TOKEN_TYPE::RANGLE:
          return builder->CreateFCmpUGT(left, right);
        case TOKEN_TYPE::GEQ:
          return builder->CreateFCmpUGE(left, right);
        case TOKEN_TYPE::LEQ:
          return builder->CreateFCmpULE(left, right);
        case TOKEN_TYPE::EQUALS:
          return builder->CreateFCmpUEQ(left, right);
        case TOKEN_TYPE::NEQUALS:
          return builder->CreateFCmpUNE(left, right);
      }
    }
    }
  }
  // Generate code for type conversions
  Value* visitTypeConvExpr(Expr* expr) override {
    Value* exp = _visitExpr(expr->getTypeConvExpr()->expr.get());
    return builder->CreateSIToFP(exp, builder->getDoubleTy());
  }

  void enterStmtVisitor() override {};
  void exitStmtVisitor() override {};
  Value* visitContinueStmt(Stmt* continueStmt) override {

  };
  Value* visitDeclarationStmt(Stmt* declarationStmt) override {
    Value* val = declarationStmt->getDeclarationStmt()->val ? _visitExpr(declarationStmt->getDeclarationStmt()->val.get()) : nullptr;
    if(declarationStmt->getDeclarationStmt()->val->isFunctionExpr()) return llvm::Constant::getNullValue(builder->getInt32Ty());

    IRBuilder<> TmpB(builder->GetInsertBlock());
    varEnv[declarationStmt->getDeclarationStmt()->name] = builder->CreateAlloca(generateType(declarationStmt->type.get()), nullptr, declarationStmt->getDeclarationStmt()->name.c_str());
    if(val) {
      builder->CreateStore(val, varEnv[declarationStmt->getDeclarationStmt()->name]);
      return val;
    }
    return llvm::Constant::getNullValue(builder->getInt32Ty());

  };
  Value* visitReturnStmt(Stmt* returnStmt) override {
    Value* returner = _visitExpr(returnStmt->getReturnStmt()->val.get());
    builder-> CreateRet(returner);
    return returner;
  };
  Value* visitYieldStmt(Stmt* yieldStmt) override {
    Value* yield = _visitExpr(yieldStmt->getYieldStmt()->val.get());
    return yield;
  };
  Value* visitExprStmt(Stmt* exprStmt) override {
    Value* ret = _visitExpr(exprStmt->getExprStmt()->val.get());
    return ret;
  };
  Value* visitClassStmt(Stmt* classStmt) override {};
  Value* visitImplStmt(Stmt* implStmt) override {};
  Value* visitTypeDef(Stmt* typeDef) override {
  };
  Value* visitVoidExpr(Expr* voidExpr) override {return llvm::ConstantInt::get(*context, llvm::APInt(1, 0, true));};
  Value* visitLiteralExpr(Expr* literalExpr) override {
    if(varEnv.find(literalExpr->getLiteralExpr()->name) == varEnv.end()) {
      std::cerr << "Couldn't find Literal Expression";
    }
    return builder->CreateLoad(generateType(literalExpr->type.get()), varEnv[literalExpr->getLiteralExpr()->name], literalExpr->getLiteralExpr()->name.c_str());
  };
  Value* visitFunctionExpr(Expr* functionExpr) override {
    std::vector<llvm::Type*> paramTypes;
    for(int i = 0; i < functionExpr->getFunctionExpr()->parameters->members.size(); ++i) {
      paramTypes.emplace_back(generateType(functionExpr->getFunctionExpr()->parameters->getInOrder(i)->type.get()));
    }
    llvm::Type* returner = generateType(functionExpr->getFunctionExpr()->returnType.get());
    auto* newFun = llvm::Function::Create(llvm::FunctionType::get(returner, paramTypes, false), llvm::Function::ExternalLinkage, functionExpr->getFunctionExpr()->name.c_str(), module);
    auto* entryBlock = llvm::BasicBlock::Create(*context, "funentry", newFun);
    builder->SetInsertPoint(entryBlock);
    _visitExpr(functionExpr->getFunctionExpr()->action.get());
    return llvm::Constant::getNullValue(builder->getInt32Ty());

  };
  Value* visitMatchExpr(Expr* matchExpr) override {
    return llvm::Constant::getNullValue(builder->getInt32Ty());
  };
  Value* visitIfExpr(Expr* ifExpr) override {
    auto* condVal = _visitExpr(ifExpr->getIfExpr()->cond.get());
    auto* parent = builder->GetInsertBlock()->getParent();
    auto* then = llvm::BasicBlock::Create(*context, "thenExpr", parent);
    llvm::BasicBlock* elseExpr = nullptr;
    Value* elseVal;
    if(ifExpr->getIfExpr()->elseExpr) {
      elseExpr = llvm::BasicBlock::Create(*context, "elseExpr");
    }
    auto* mergeAfter = llvm::BasicBlock::Create(*context, "continued");
    builder->CreateCondBr(condVal, then, elseExpr ? elseExpr : mergeAfter);
    builder->SetInsertPoint(then);
    auto* thenVal = _visitExpr(ifExpr->getIfExpr()->thenExpr.get());
    builder->CreateBr(mergeAfter);
    then = builder->GetInsertBlock();
    if(elseExpr) {
      parent->insert(parent->end(), elseExpr);
      builder->SetInsertPoint(elseExpr);
      elseVal = _visitExpr(ifExpr->getIfExpr()->elseExpr.get());

      builder->CreateBr(mergeAfter);
      elseExpr = builder->GetInsertBlock();
    }
    parent->insert(parent->end(), mergeAfter);
    builder->SetInsertPoint(mergeAfter);
    if(elseExpr) {
      auto* phi =
          builder->CreatePHI(thenVal->getType(), 2, "ifstmt");
      phi->addIncoming(thenVal, then);
        phi->addIncoming(elseVal, elseExpr);
      return phi;
    }
    return thenVal;

  };
  Value* visitBlockExpr(Expr* blockExpr) override {
    auto* parent = builder->GetInsertBlock()->getParent();
    auto* block = llvm::BasicBlock::Create(*context, "block");
    builder->CreateBr(block);
    parent->insert(parent->end(), block);
    builder->SetInsertPoint(block);
    for(auto& stmt : blockExpr->getBlockExpr()->stmts) {
      Value* val = _visitStmt(stmt.get());
      if(stmt->isYieldStmt()) return val;
    }
    return llvm::Constant::getNullValue(builder->getInt32Ty());
  };
  Value* visitForExpr(Expr* forExpr) override {};
  Value* visitWhileExpr(Expr* whileExpr) override {
    auto* parent = builder->GetInsertBlock()->getParent();
    auto* cond = _visitExpr(whileExpr->getWhileExpr()->cond.get());
    llvm::BasicBlock* loopBody = llvm::BasicBlock::Create(*context, "loop");
    llvm::BasicBlock* loopEnd = llvm::BasicBlock::Create(*context, "end");
    builder->CreateCondBr(cond, loopBody, loopEnd);
    parent->insert(parent->end(), loopBody);
    builder->SetInsertPoint(loopBody);
    llvm::Value* val = _visitExpr(whileExpr->getWhileExpr()->body.get());
    auto* currentInsert = builder->GetInsertBlock();
    cond = _visitExpr(whileExpr->getWhileExpr()->cond.get());
    builder->CreateCondBr(cond, loopBody, loopEnd);
    parent->insert(parent->end(), loopEnd);
    builder->SetInsertPoint(loopEnd);
    return val;


  };
  Value* visitGetExpr(Expr* getExpr) override {};
  Value* visitCallExpr(Expr* callExpr) override {
    auto* func = module->getFunction("printf");
    std::vector<Value*> arguments;
    for(auto& param : callExpr->getCallExpr()->params) {
      arguments.emplace_back(_visitExpr(param.get()));
    }
    return builder->CreateCall(func, arguments);
  };
  // Enter a visitor (no implementation)
  void enterExprVisitor() override {}
  // Exit a visitor (no implementation)
  void exitExprVisitor() override {}
  // Use default copy and destructor operations
  CodeGen(CodeGen&&) = default;
  CodeGen(const CodeGen&) = default;
  CodeGen& operator=(CodeGen&&) = default;
  CodeGen& operator=(const CodeGen&) = default;

  ~CodeGen() = default;

 private:
  llvm::Type* generateType(Type* exprType) {
    if(exprType->isAliasType()) {
      return generateType(exprType->getAliasType()->type.get());
    } else if(exprType->isBottomType()) {
      switch (exprType->getBottomType()) {
        case BottomType::INT:
          return builder->getInt32Ty();
        case BottomType::CHAR:
          return builder->getInt8Ty();
        case BottomType::BOOL:
        return builder->getInt1Ty();
        case BottomType::FLOAT:
          return builder->getFloatTy();
        case BottomType::VOID:
          return builder->getVoidTy();
        case BottomType::SELF:
          std::cerr << "Error: Unsubstantiated Type.\n";
          break;
      }
    }
    return builder->getInt32Ty();
  }
};

#endif  // INCLUDE_SRC_CODEGEN_H_
