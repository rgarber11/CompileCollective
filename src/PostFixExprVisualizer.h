// Copyright (c) 2024 Compile Collective. All Rights Reserved.

#ifndef SENIORPROJECT_POSTFIXEXPRVISUALIZER_H
#define SENIORPROJECT_POSTFIXEXPRVISUALIZER_H
#include <iostream>

#include "expr.h"
// Print postfix expressions to standard output
class PostFixExprVisualizer : public Visitor<void> {
  bool first = true;
  // Visit both children of a binary expression
  void visitBinaryExpr(Expr *expr) override {
    _visit(expr->getBinary()->left.get());
    _visit(expr->getBinary()->right.get());
    std::cout << ' ' << debugTokenTypes(expr->getBinary()->op);
  }
  // Visit an integer expression, adding a space if not first
  void visitIntExpr(Expr *expr) override {
    if (first) {
      first = false;
      std::cout << expr->getInt();
    } else {
      std::cout << ' ' << expr->getInt();
    }
  }
  // Visit a prefix expression, adding the correct token
  void visitPrefixExpr(Expr *expr) override {
    _visit(expr->getPrefix()->expr.get());
    std::cout << ' ' << debugTokenTypes(expr->getPrefix()->op);
  }
  // Visit a float expression, adding a space if not first
  void visitFloatExpr(Expr *expr) override {
    if (first) {
      first = false;
      std::cout << expr->getFloat();
    } else {
      std::cout << ' ' << expr->getFloat();
    }
  }
  // Enter a postfix expr visitor (no implementation)
  void enterVisitor() override {}
  // Exit a postfix expr visitor
  void exitVisitor() override {
    // End the line and reset first
    first = true;
    std::cout << '\n';
  }
  // Handle implicit type conversion
  void visitImplicitTypeConvExpr(Expr *expr) override {
    _visit(expr->getImplicitTypeConvExpr()->expr.get());
  }
};

#endif  // SENIORPROJECT_POSTFIXEXPRVISUALIZER_H
