// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef INCLUDE_SRC_INT_OPTIMIZER_H_
#define INCLUDE_SRC_INT_OPTIMIZER_H_

#include <memory>
#include <utility>

#include "expr.h"
using std::unique_ptr, std::pair;
struct IntOptimizer : public Visitor<Expr*> {
 private:
  Expr* visitIntExpr(Expr* expr) override { return expr; }
  Expr* visitFloatExpr(Expr* expr) override { return expr; }
  Expr* visitBinaryExpr(Expr* expr) override {
    expr->getBinary()->left.reset(_visit(expr->getBinary()->left.release()));
    expr->getBinary()->right.reset(_visit(expr->getBinary()->right.release()));
    return expr;
  }
  Expr* visitPrefixExpr(Expr* expr2) override {
    expr2->getPrefix()->expr.reset(_visit(expr2->getPrefix()->expr.release()));
    if (expr2->getPrefix()->expr->isInt()) {
      int prevVal = expr2->getPrefix()->expr->getInt();
      expr2->innerExpr.emplace<IntExpr>(IntExpr{-1 * prevVal});
      return expr2;
    }
    return expr2;
  }
  void enterVisitor() override {}
  void exitVisitor() override {}
};

#endif  // INCLUDE_SRC_INT_OPTIMIZER_H_
