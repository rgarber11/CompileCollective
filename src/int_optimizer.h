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
  Expr* visitBinaryExpr(Expr* expr) override {
    _visit(expr->getBinary()->left.get());
    _visit(expr->getBinary()->right.get());
    return expr;
  }
  Expr* visitPrefixExpr(Expr* expr2) override {
    if (expr2->getPrefix()->expr->isInt()) {
      int prevVal = expr2->getPrefix()->expr->getInt();
      expr2->innerExpr.emplace<IntExpr>(IntExpr{-1 * prevVal});
      return expr2;
    }
    _visit(expr2->getPrefix()->expr.get());
    return expr2;
  }
  void enterVisitor() override {}
  void exitVisitor() override {}
};

#endif  // INCLUDE_SRC_INT_OPTIMIZER_H_
