// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#include "expr.h"

BinaryExpr::BinaryExpr(const Token &opToken) : op(opToken.type) {}
BinaryExpr::BinaryExpr(const BinaryExpr &binaryExpr)
    : left(binaryExpr.left ? binaryExpr.left->clone() : nullptr),
      right(binaryExpr.right ? binaryExpr.right->clone() : nullptr),
      op(binaryExpr.op) {}
BinaryExpr::BinaryExpr(BinaryExpr &&binaryExpr) noexcept
    : left(binaryExpr.left ? binaryExpr.left->clone() : nullptr),
      right(binaryExpr.right ? binaryExpr.right->clone() : nullptr),
      op(binaryExpr.op) {}
PrefixExpr::PrefixExpr(const PrefixExpr &prefixExpr)
    : expr(prefixExpr.expr ? prefixExpr.expr->clone() : nullptr),
      op(prefixExpr.op) {}
PrefixExpr::PrefixExpr(const Token &opToken) : op(opToken.type) {}
PrefixExpr::PrefixExpr(PrefixExpr &&prefixExpr) noexcept
    : expr(prefixExpr.expr ? prefixExpr.expr->clone() : nullptr),
      op(prefixExpr.op) {}
Expr Expr::makeBinary(const Token &op) {
  return Expr{op.sourceLocation, BinaryExpr{op}};
}
Expr Expr::makePrefix(const Token &op) {
  return Expr{op.sourceLocation, PrefixExpr{op}};
}
Expr Expr::makeInt(const Token &op, int num) {
  return Expr{op.sourceLocation, IntExpr{num}};
}
Expr Expr::makeFloat(const Token &op, double num) {
  return Expr{op.sourceLocation, FloatExpr{num}};
}
