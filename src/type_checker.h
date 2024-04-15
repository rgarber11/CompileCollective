// Copyright (c) 2024 Compile Collective. All Rights Reserved.

#ifndef SENIORPROJECT_TYPE_CHECKER_H
#define SENIORPROJECT_TYPE_CHECKER_H
#include <memory>
#include <utility>

#include "environment.h"
#include "expr.h"
#include "stmt.h"
#include "types.h"

struct TypeChecker : public ExprVisitor<Expr*>, StmtVisitor<void> {
  TypeChecker(Environment* program) : program(program){};
  void visitDeclarationStmt(Stmt* stmt) override {
    if (stmt->getDeclarationStmt()->val) {
      _visitExpr(stmt->getDeclarationStmt()->val.get());

      if (!stmt->type) {
        stmt->type = stmt->getDeclarationStmt()->val->type;
      } else {
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
  };
  Expr* visitIntExpr(Expr* expr) override { return expr; }
  Expr* visitFloatExpr(Expr* expr) override { return expr; }
  Expr* visitCharExpr(Expr* expr) override { return expr; }
  Expr* visitBoolExpr(Expr* expr) override { return expr; }
  Expr* visitStringExpr(Expr* expr) override { return expr; }
  Expr* visitBinaryExpr(Expr* expr) override {
    _visitExpr(expr->getBinaryExpr()->left.get());
    _visitExpr(expr->getBinaryExpr()->right.get());
    if (expr->getBinaryExpr()->left->type ==
        expr->getBinaryExpr()->right->type) {
      expr->type = expr->getBinaryExpr()->left->type;
    } else if (expr->getBinaryExpr()->left->type == TOKEN_TYPE::FLOAT) {
      expr->type = TOKEN_TYPE::FLOAT;
      auto newRight = std::make_unique<Expr>(Expr::makeImplicitTypeConv(
          expr->sourceLocation, program->bottomTypes.intType,
          program->bottomTypes.floatType));
      newRight->getImplicitTypeConvExpr()->expr =
          std::move(expr->getBinaryExpr()->right);
      expr->getBinaryExpr()->right = std::move(newRight);
    } else {
      expr->type = TOKEN_TYPE::FLOAT;
      auto newLeft = std::make_unique<Expr>(Expr::makeImplicitTypeConv(
          expr->sourceLocation, TOKEN_TYPE::INT, TOKEN_TYPE::FLOAT));
      newLeft->getImplicitTypeConvExpr()->expr =
          std::move(expr->getBinaryExpr()->left);
      expr->getBinaryExpr()->left = std::move(newLeft);
    }
    return expr;
  }
  Expr* visitPrefixExpr(Expr* expr2) override {
    _visit(expr2->getPrefixExpr()->expr.get());
    expr2->type = expr2->getPrefixExpr()->expr->type;
    if (expr2->getPrefixExpr()->expr->isInt()) {
      int prevVal = expr2->getPrefixExpr()->expr->getInt();
      expr2->innerExpr.emplace<IntExpr>(IntExpr{-1 * prevVal});
      return expr2;
    } else if (expr2->getPrefixExpr()->expr->isFloat()) {
      double prevVal = expr2->getPrefixExpr()->expr->getFloat();
      expr2->innerExpr.emplace<FloatExpr>(FloatExpr{-1 * prevVal});
      return expr2;
    }
    return expr2;
  }
  Expr* visitImplicitTypeConvExpr(Expr* expr) override { return expr; }
  void enterExprVisitor() override {}
  void exitExprVisitor() override {}

 private:
  Environment* program;
};
#endif  // SENIORPROJECT_TYPE_CHECKER_H
