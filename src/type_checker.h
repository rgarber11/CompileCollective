// Copyright (c) 2024 Compile Collective. All Rights Reserved.

#ifndef SENIORPROJECT_TYPE_CHECKER_H
#define SENIORPROJECT_TYPE_CHECKER_H
#include <memory>
#include <utility>

#include "environment.h"
#include "expr.h"
#include "parser.h"
#include "stmt.h"
#include "token.h"
#include "types.h"

// Type checker - ensure type compatibility
struct TypeChecker : public ExprVisitor<Expr*>, StmtVisitor<void> {
  // Constructor - take in an environment
  TypeChecker(Environment* program) : program(program){};
  // Check a declaration statement
  void visitDeclarationStmt(Stmt* stmt) override {
    if (stmt->getDeclarationStmt()->val) {
      _visitExpr(stmt->getDeclarationStmt()->val.get());
      if (stmt->type->isAliasType() && !stmt->type->getAliasType()->type) {
        stmt->type =
            program->getMember(stmt->type->getAliasType()->alias)->type;
      }
      if (!stmt->type) {
        stmt->type = stmt->getDeclarationStmt()->val->type;
      } else {
        // If conversion is explicit or impossible, output error
        switch (stmt->type->isConvertible(
            stmt->getDeclarationStmt()->val->type.get())) {
          case Convert::SAME:
            break;
          case Convert::IMPLICIT:
            break;
          case Convert::EXPLICIT:
          case Convert::FALSE:
            std::cerr << "These don't fit together";
        }
      }
    }
  }
  // Enter and exit statement visitors (no implementation)
  void enterStmtVisitor() override {}
  void exitStmtVisitor() override {}
  // Check a continue statement (no implementation)
  void visitContinueStmt(Stmt* continueStmt) override { return; }
  // Check a return statement
  void visitReturnStmt(Stmt* returnStmt) override {
    _visitExpr(returnStmt->getReturnStmt()->val.get());
  }
  // Check a yield statement
  void visitYieldStmt(Stmt* yieldStmt) override {
    _visitExpr(yieldStmt->getYieldStmt()->val.get());
  }
  // Check an expression statement
  void visitExprStmt(Stmt* exprStmt) override {
    _visitExpr(exprStmt->getExprStmt()->val.get());
  }
  // Check a class statement
  void visitClassStmt(Stmt* classStmt) override {
    // Check all parameters
    for (auto& i : classStmt->getClassStmt()->parameters) {
      _visitStmt(&i);
    }
  }
  // No implementations necessary
  void visitImplStmt(Stmt* implStmt) override {}
  void visitTypeDef(Stmt* typeDef) override { return; }
  Expr* visitIntExpr(Expr* expr) override { return expr; }
  Expr* visitFloatExpr(Expr* expr) override { return expr; }
  Expr* visitCharExpr(Expr* expr) override { return expr; }
  Expr* visitBoolExpr(Expr* expr) override { return expr; }
  Expr* visitStringExpr(Expr* expr) override { return expr; }
  // Check binary expression
  Expr* visitBinaryExpr(Expr* expr) override {
    // Check both sides
    _visitExpr(expr->getBinaryExpr()->left.get());
    _visitExpr(expr->getBinaryExpr()->right.get());
    switch (expr->getBinaryExpr()->op) {
      case TOKEN_TYPE::PLUS:
      case TOKEN_TYPE::MINUS:
      case TOKEN_TYPE::SLASH:
      case TOKEN_TYPE::STAR: {
        // +-*/ operations require int or float
        if (expr->getBinaryExpr()->left->type != program->bottomTypes.intType ||
            expr->getBinaryExpr()->left->type != program->bottomTypes.floatType)
          return nullptr;
        if (expr->getBinaryExpr()->right->type !=
                program->bottomTypes.intType ||
            expr->getBinaryExpr()->right->type !=
                program->bottomTypes.floatType)
          return nullptr;
        auto convert = expr->getBinaryExpr()->left->type->isConvertible(
            expr->getBinaryExpr()->right->type.get());
        if (convert == Convert::SAME) break;
        if (convert == Convert::IMPLICIT) {
          auto typeConv = std::make_unique<Expr>(
              Expr(expr->getBinaryExpr()->right->sourceLocation,
                   expr->getBinaryExpr()->left->type,
                   TypeConvExpr{true, expr->getBinaryExpr()->right->type,
                                expr->getBinaryExpr()->left->type}));
          typeConv->getTypeConvExpr()->expr =
              std::move(expr->getBinaryExpr()->right);
          expr->getBinaryExpr()->right = std::move(typeConv);
        } else {
          convert = expr->getBinaryExpr()->right->type->isConvertible(
              expr->getBinaryExpr()->left->type.get());
          if (convert == Convert::IMPLICIT) {
            auto typeConv = std::make_unique<Expr>(
                Expr(expr->getBinaryExpr()->left->sourceLocation,
                     expr->getBinaryExpr()->right->type,
                     TypeConvExpr{true, expr->getBinaryExpr()->left->type,
                                  expr->getBinaryExpr()->right->type}));
            typeConv->getTypeConvExpr()->expr =
                std::move(expr->getBinaryExpr()->left);
            expr->getBinaryExpr()->left = std::move(typeConv);
          } else {
            return nullptr;
          }
        }
        expr->type = expr->getBinaryExpr()->left->type;
        break;
      }
      case TOKEN_TYPE::LANGLE:
      case TOKEN_TYPE::RANGLE:
      case TOKEN_TYPE::GEQ:
      case TOKEN_TYPE::LEQ: {
        // Comparison operators require int or float
        if (expr->getBinaryExpr()->left->type != program->bottomTypes.intType ||
            expr->getBinaryExpr()->left->type != program->bottomTypes.floatType)
          return nullptr;
        if (expr->getBinaryExpr()->right->type !=
                program->bottomTypes.intType ||
            expr->getBinaryExpr()->right->type !=
                program->bottomTypes.floatType)
          return nullptr;
        auto convert = expr->getBinaryExpr()->left->type->isConvertible(
            expr->getBinaryExpr()->right->type.get());
        if (convert == Convert::SAME) break;
        if (convert == Convert::IMPLICIT) {
          auto typeConv = std::make_unique<Expr>(
              Expr(expr->getBinaryExpr()->right->sourceLocation,
                   expr->getBinaryExpr()->left->type,
                   TypeConvExpr{true, expr->getBinaryExpr()->right->type,
                                expr->getBinaryExpr()->left->type}));
          typeConv->getTypeConvExpr()->expr =
              std::move(expr->getBinaryExpr()->right);
          expr->getBinaryExpr()->right = std::move(typeConv);
        } else {
          convert = expr->getBinaryExpr()->right->type->isConvertible(
              expr->getBinaryExpr()->left->type.get());
          if (convert == Convert::IMPLICIT) {
            auto typeConv = std::make_unique<Expr>(
                Expr(expr->getBinaryExpr()->left->sourceLocation,
                     expr->getBinaryExpr()->right->type,
                     TypeConvExpr{true, expr->getBinaryExpr()->left->type,
                                  expr->getBinaryExpr()->right->type}));
            typeConv->getTypeConvExpr()->expr =
                std::move(expr->getBinaryExpr()->left);
            expr->getBinaryExpr()->left = std::move(typeConv);
          } else {
            return nullptr;
          }
        }
        expr->type = program->bottomTypes.boolType;
        break;
      }
      case TOKEN_TYPE::MOD:
      case TOKEN_TYPE::LSHIFT:
      case TOKEN_TYPE::RSHIFT:
      case TOKEN_TYPE::BITAND:
      case TOKEN_TYPE::BITOR:
      case TOKEN_TYPE::XOR:
      case TOKEN_TYPE::RANGE:
      case TOKEN_TYPE::INCRANGE:
        // Modulo, range, and bit operations require int
        if (expr->getBinaryExpr()->left->type != program->bottomTypes.intType)
          return nullptr;
        if (expr->getBinaryExpr()->right->type != program->bottomTypes.intType)
          return nullptr;
        expr->type = program->bottomTypes.intType;
        break;
      case TOKEN_TYPE::ASSIGN:
        // Assignment is safe
        break;
      case TOKEN_TYPE::OR:
      case TOKEN_TYPE::AND:
      case TOKEN_TYPE::EQUALS:
      case TOKEN_TYPE::NEQUALS:
        // Logical and equality operators require bool
        if (expr->getBinaryExpr()->left->type != program->bottomTypes.boolType)
          return nullptr;
        if (expr->getBinaryExpr()->right->type != program->bottomTypes.boolType)
          return nullptr;
        expr->type = program->bottomTypes.boolType;
        break;
    }
    return expr;
  }
  // Check prefix expression
  Expr* visitPrefixExpr(Expr* expr2) override {
    _visitExpr(expr2->getPrefixExpr()->expr.get());
    switch (expr2->getPrefixExpr()->op) {
      case TOKEN_TYPE::MINUS:
        // - requires int or float
        if (expr2->getPrefixExpr()->expr->isIntExpr()) {
          expr2->innerExpr =
              IntExpr{-1 * expr2->getPrefixExpr()->expr->getInt()};
          expr2->type = program->bottomTypes.intType;
          return expr2;
        } else if (expr2->getPrefixExpr()->expr->isFloatExpr()) {
          expr2->innerExpr =
              FloatExpr(-1 * expr2->getPrefixExpr()->expr->getFloatExpr()->val);
          expr2->type = program->bottomTypes.floatType;
          return expr2;
        }
        if (expr2->getPrefixExpr()->expr->type !=
                program->bottomTypes.intType ||
            expr2->getPrefixExpr()->expr->type !=
                program->bottomTypes.floatType)
          return nullptr;
        break;
      case TOKEN_TYPE::NOT:
        // ! requires bool or int
        if (expr2->getPrefixExpr()->expr->isIntExpr()) {
          expr2->innerExpr = IntExpr{~expr2->getPrefixExpr()->expr->getInt()};
          expr2->type = program->bottomTypes.intType;
          return expr2;
        } else if (expr2->getPrefixExpr()->expr->isBoolExpr()) {
          expr2->innerExpr =
              BoolExpr(!expr2->getPrefixExpr()->expr->getBoolExpr()->val);
          expr2->type = program->bottomTypes.boolType;
          return expr2;
        }
        if (expr2->getPrefixExpr()->expr->type !=
                program->bottomTypes.intType ||
            expr2->getPrefixExpr()->expr->type !=
                program->bottomTypes.floatType)
          return nullptr;
        break;
    }
    expr2->type = expr2->getPrefixExpr()->expr->type;
    return expr2;
  }
  // Check type conversion expression (no implementation)
  Expr* visitTypeConvExpr(Expr* expr) override { return expr; }
  // Check literal expression
  Expr* visitLiteralExpr(Expr* literalExpr) override {
    literalExpr->type =
        program->getMember(literalExpr->getLiteralExpr()->name)
            ? program->getMember(literalExpr->getLiteralExpr()->name)->type
            : nullptr;
    return literalExpr;
  }
  // Enter and exit visitor (no implementation)
  void enterExprVisitor() override {}
  void exitExprVisitor() override {}
  // Visit function and match expression (no implementation)
  Expr* visitFunctionExpr(Expr* functionExpr) override {}
  Expr* visitMatchExpr(Expr* matchExpr) override {}
  // Visit if expression
  Expr* visitIfExpr(Expr* ifExpr) override {
    visitExpr(ifExpr->getIfExpr()->cond.get());
    // Condition must be bool
    if (ifExpr->getIfExpr()->cond->type != program->bottomTypes.boolType) {
      std::cerr << "Big Problem!\n";
      return nullptr;
    }
    visitExpr(ifExpr->getIfExpr()->thenExpr.get());
    if (!ifExpr->getIfExpr()->elseExpr) {
      ifExpr->type = ifExpr->getIfExpr()->thenExpr->type;
      return ifExpr;
    }
    visitExpr(ifExpr->getIfExpr()->elseExpr.get());
    if (ifExpr->getIfExpr()->thenExpr->type->isConvertible(
            ifExpr->getIfExpr()->elseExpr->type.get()) == Convert::SAME) {
      ifExpr->type = ifExpr->getIfExpr()->thenExpr->type;
      return ifExpr;
    } else if (ifExpr->getIfExpr()->thenExpr->type ==
               program->bottomTypes.voidType) {
      if (ifExpr->getIfExpr()->elseExpr->type->isOptionalType()) {
        ifExpr->type = ifExpr->getIfExpr()->elseExpr->type;
        return ifExpr;
      } else {
        ifExpr->type = std::make_shared<Type>(
            OptionalType(ifExpr->getIfExpr()->elseExpr->type),
            std::vector<std::shared_ptr<Impl>>{});
        auto typeConv = std::make_unique<Expr>(
            ifExpr->getIfExpr()->elseExpr->sourceLocation, ifExpr->type,
            TypeConvExpr{true, ifExpr->getIfExpr()->elseExpr->type,
                         ifExpr->type});
        typeConv->getTypeConvExpr()->expr =
            std::move(ifExpr->getIfExpr()->elseExpr);
        ifExpr->getIfExpr()->elseExpr = std::move(typeConv);
        return ifExpr;
      }
    } else if (ifExpr->getIfExpr()->elseExpr->type ==
               program->bottomTypes.voidType) {
      if (ifExpr->getIfExpr()->thenExpr->type->isOptionalType()) {
        ifExpr->type = ifExpr->getIfExpr()->thenExpr->type;
        return ifExpr;
      } else {
        ifExpr->type = std::make_shared<Type>(
            OptionalType(ifExpr->getIfExpr()->thenExpr->type),
            std::vector<std::shared_ptr<Impl>>{});
        auto typeConv = std::make_unique<Expr>(
            ifExpr->getIfExpr()->thenExpr->sourceLocation, ifExpr->type,
            TypeConvExpr{true, ifExpr->getIfExpr()->thenExpr->type,
                         ifExpr->type});
        typeConv->getTypeConvExpr()->expr =
            std::move(ifExpr->getIfExpr()->thenExpr);
        ifExpr->getIfExpr()->thenExpr = std::move(typeConv);
        return ifExpr;
      }
    } else if (ifExpr->getIfExpr()->thenExpr->type->isConvertible(
                   ifExpr->getIfExpr()->elseExpr->type.get()) ==
               Convert::IMPLICIT) {
    } else if (ifExpr->getIfExpr()->elseExpr->type->isConvertible(
                   ifExpr->getIfExpr()->thenExpr->type.get()) ==
               Convert::IMPLICIT) {
    } else {
    }
    return ifExpr;
  }
  // Check block expression
  Expr* visitBlockExpr(Expr* blockExpr) override {
    // A block that does not yield requires void type
    if (!blockExpr->getBlockExpr()->yields) {
      blockExpr->type = program->bottomTypes.voidType;
      return blockExpr;
    }
    return blockExpr;
  }
  // Check a for expression
  Expr* visitForExpr(Expr* forExpr) override {
    // Visit all statements and body expression
    for (int i = 0; i < forExpr->getForExpr()->env->members.size(); ++i) {
      _visitStmt(forExpr->getForExpr()->env->getInOrder(i));
    }
    _visitExpr(forExpr->getForExpr()->body.get());
    forExpr->type = forExpr->getForExpr()->body->type;
    return forExpr;
  }
  // Check while expression
  Expr* visitWhileExpr(Expr* whileExpr) override {
    _visitExpr(whileExpr->getWhileExpr()->cond.get());
    // Condition must be bool
    if (whileExpr->getWhileExpr()->cond->type !=
        program->bottomTypes.boolType) {
      std::cerr << "Invalid Type.";
      return nullptr;
    }
    _visitExpr(whileExpr->getWhileExpr()->body.get());
    whileExpr->type = whileExpr->getWhileExpr()->body->type;
    return whileExpr;
  }
  // Check get expression
  Expr* visitGetExpr(Expr* getExpr) override {
    // Ensure object element is defined
    if (program->isRedeclaration(getExpr->getGetExpr()->name.name) !=
        Environment::REDECLARATION_STATES::REDECLARATION) {
      std::cerr << "Element not defined on object\n";
      return nullptr;
    }
    getExpr->type = program->getMember(getExpr->getGetExpr()->name.name)->type;
    return getExpr;
  }
  // Check call expression
  Expr* visitCallExpr(Expr* callExpr) override {
    _visitExpr(callExpr->getCallExpr()->expr.get());
    if (callExpr->getCallExpr()->expr->isLiteralExpr() &&
        callExpr->getCallExpr()->expr->getLiteralExpr()->name == "convert") {
      if (callExpr->getCallExpr()->params.size() != 2 ||
          !callExpr->getCallExpr()->params[0]->isLiteralExpr())
        return nullptr;
      Environment getType =
          Parser(
              Lexer(callExpr->getCallExpr()->params[0]->getLiteralExpr()->name),
              program->generateInnerEnvironment())
              .parse(Parser::parser::TYPE);
      auto explicitType =
          getType.getMember("$TypeCheckerName")->getTypeDef()->type;
      _visitExpr(callExpr->getCallExpr()->params[1].get());
      if (callExpr->getCallExpr()->params[1]->type->isConvertible(
              explicitType.get()) == Convert::FALSE)
        return nullptr;
      auto storage = std::move(callExpr->getCallExpr()->params[1]);
      auto typeConv = TypeConvExpr{false, storage->type, explicitType};
      typeConv.expr = std::move(storage);
      callExpr->innerExpr = std::move(typeConv);
      return callExpr;
    } else if (callExpr->getCallExpr()->expr->type->isStructType()) {
      for (int i = 0; i < callExpr->getCallExpr()->params.size(); ++i) {
        // Visit each parameter
        Expr* expr = callExpr->getCallExpr()->params[i].get();
        _visitExpr(expr);
        switch (callExpr->getCallExpr()
                    ->expr->type->getStructType()
                    ->types[i]
                    .type->isConvertible(expr->type.get())) {
          case Convert::SAME:
            break;
          case Convert::IMPLICIT: {
            std::shared_ptr<Type> newType = callExpr->getCallExpr()
                                                ->expr->type->getFunctionType()
                                                ->parameters[i];
            auto typeConv =
                std::make_unique<Expr>(expr->sourceLocation, newType,
                                       TypeConvExpr{true, expr->type, newType});
            typeConv->getTypeConvExpr()->expr =
                std::move(callExpr->getCallExpr()->params[i]);
            callExpr->getCallExpr()->params[i] = std::move(typeConv);
            break;
          }
          case Convert::EXPLICIT:
          case Convert::FALSE:
            return nullptr;
        }
      }
      callExpr->type = callExpr->getCallExpr()->expr->type;
    } else if (callExpr->getCallExpr()->expr->type->isFunctionType()) {
      if (callExpr->getCallExpr()
              ->expr->type->getFunctionType()
              ->parameters.size() != callExpr->getCallExpr()->params.size()) {
        // Ensure arity matches
        std::cerr << "Arity doesn't match.";
        return nullptr;
      }
      for (int i = 0; i < callExpr->getCallExpr()->params.size(); ++i) {
        Expr* expr = callExpr->getCallExpr()->params[i].get();
        // Visit each parameter
        _visitExpr(expr);
        switch (callExpr->getCallExpr()
                    ->expr->type->getFunctionType()
                    ->parameters[i]
                    ->isConvertible(expr->type.get())) {
          case Convert::SAME:
            break;
          case Convert::IMPLICIT: {
            std::shared_ptr<Type> newType = callExpr->getCallExpr()
                                                ->expr->type->getFunctionType()
                                                ->parameters[i];
            auto typeConv =
                std::make_unique<Expr>(expr->sourceLocation, newType,
                                       TypeConvExpr{true, expr->type, newType});
            typeConv->getTypeConvExpr()->expr =
                std::move(callExpr->getCallExpr()->params[i]);
            callExpr->getCallExpr()->params[i] = std::move(typeConv);
            break;
          }
          case Convert::EXPLICIT:
          case Convert::FALSE:
            return nullptr;
        }
      }
      callExpr->type =
          callExpr->getCallExpr()->expr->type->getFunctionType()->returner;
      return callExpr;
    } else if (callExpr->getCallExpr()->expr->type->isListType() &&
               callExpr->getCallExpr()->params.size() == 1) {
      _visitExpr(callExpr->getCallExpr()->params.front().get());
      if (callExpr->getCallExpr()->params.front()->type !=
          program->bottomTypes.intType) {
        // Front parameter must be int
        std::cerr << "Bad Index!";
        return nullptr;
      }
      callExpr->type = callExpr->getCallExpr()->expr->type->getListType()->type;
      return callExpr;
    }
    return nullptr;
  }

 private:
  Environment* program;
};
#endif  // SENIORPROJECT_TYPE_CHECKER_H
