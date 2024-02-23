// Copyright (c) 2024 Compile Collective. All Rights Reserved.

#ifndef SENIORPROJECT_POSTFIXEXPRVISUALIZER_H
#define SENIORPROJECT_POSTFIXEXPRVISUALIZER_H
#include <iostream>

#include "expr.h"
class PostFixExprVisualizer : public Visitor<void> {
  bool first = true;
  void visitBinaryExpr(Expr *expr) override {
    _visit(expr->getBinary()->left.get());
    _visit(expr->getBinary()->right.get());
    std::cout << ' ' << debugTokenTypes(expr->getBinary()->op);
  }
  void visitIntExpr(Expr *expr) override {
    if (first) {
      first = false;
      std::cout << expr->getInt();
    } else {
      std::cout << ' ' << expr->getInt();
    }
  }
  void visitPrefixExpr(Expr *expr) override {
    _visit(expr->getPrefix()->expr.get());
    std::cout << ' ' << debugTokenTypes(expr->getPrefix()->op);
  }
  void visitFloatExpr(Expr *expr) override {
    if (first) {
      first = false;
      std::cout << expr->getFloat();
    } else {
      std::cout << ' ' << expr->getFloat();
    }
  }
  void enterVisitor() override {}
  void exitVisitor() override {
    first = true;
    std::cout << '\n';
  }
};

#endif  // SENIORPROJECT_POSTFIXEXPRVISUALIZER_H
