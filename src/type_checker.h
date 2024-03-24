// Copyright (c) 2024 Compile Collective. All Rights Reserved.

#ifndef SENIORPROJECT_TYPE_CHECKER_H
#define SENIORPROJECT_TYPE_CHECKER_H
#include <memory>
#include <utility>

#include "expr.h"

struct TypeChecker : public Visitor<Expr*> {
  Expr* visitIntExpr(Expr* expr) override { return expr; }
  Expr* visitFloatExpr(Expr* expr) override { return expr; }
  Expr* visitBinaryExpr(Expr* expr) override {
    _visit(expr->getBinary()->left.get());
    _visit(expr->getBinary()->right.get());
    if (expr->getBinary()->left->type == expr->getBinary()->right->type) {
      expr->type = expr->getBinary()->left->type;
    } else if (expr->getBinary()->left->type == TOKEN_TYPE::FLOAT) {
      expr->type = TOKEN_TYPE::FLOAT;
      auto newRight = std::make_unique<Expr>(Expr::makeImplicitTypeConv(
          expr->sourceLocation, TOKEN_TYPE::INT, TOKEN_TYPE::FLOAT));
      newRight->getImplicitTypeConvExpr()->expr =
          std::move(expr->getBinary()->right);
      expr->getBinary()->right = std::move(newRight);
    } else {
      expr->type = TOKEN_TYPE::FLOAT;
      auto newLeft = std::make_unique<Expr>(Expr::makeImplicitTypeConv(
          expr->sourceLocation, TOKEN_TYPE::INT, TOKEN_TYPE::FLOAT));
      newLeft->getImplicitTypeConvExpr()->expr =
          std::move(expr->getBinary()->left);
      expr->getBinary()->left = std::move(newLeft);
    }
    return expr;
  }
  Expr* visitPrefixExpr(Expr* expr2) override {
    _visit(expr2->getPrefix()->expr.get());
    expr2->type = expr2->getPrefix()->expr->type;
    if (expr2->getPrefix()->expr->isInt()) {
      int prevVal = expr2->getPrefix()->expr->getInt();
      expr2->innerExpr.emplace<IntExpr>(IntExpr{-1 * prevVal});
      return expr2;
    } else if (expr2->getPrefix()->expr->isFloat()) {
      double prevVal = expr2->getPrefix()->expr->getFloat();
      expr2->innerExpr.emplace<FloatExpr>(FloatExpr{-1 * prevVal});
      return expr2;
    }
    return expr2;
  }
  Expr* visitImplicitTypeConvExpr(Expr* expr) override { return expr; }
  void enterVisitor() override {}
  void exitVisitor() override {}
};
#endif  // SENIORPROJECT_TYPE_CHECKER_H
