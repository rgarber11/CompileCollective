// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#include "expr.h"

BinaryExpr::BinaryExpr(const Token &opToken) : op(opToken.type) {}
BinaryExpr::BinaryExpr(const BinaryExpr &binaryExpr)
    : op(binaryExpr.op),
      left(binaryExpr.left ? binaryExpr.left->clone() : nullptr),
      right(binaryExpr.right ? binaryExpr.right->clone() : nullptr) {}
BinaryExpr::BinaryExpr(BinaryExpr &&binaryExpr) noexcept
    : op(binaryExpr.op),
      left(binaryExpr.left ? binaryExpr.left->clone() : nullptr),
      right(binaryExpr.right ? binaryExpr.right->clone() : nullptr) {}
PrefixExpr::PrefixExpr(const PrefixExpr &prefixExpr)
    : op(prefixExpr.op),
      expr(prefixExpr.expr ? prefixExpr.expr->clone() : nullptr) {}
PrefixExpr::PrefixExpr(const Token &opToken) : op(opToken.type) {}
PrefixExpr::PrefixExpr(PrefixExpr &&prefixExpr) noexcept
    : op(prefixExpr.op),
      expr(prefixExpr.expr ? prefixExpr.expr->clone() : nullptr) {}
Expr Expr::makeBinary(const Token &op) {
  return Expr{op.sourceLocation, BinaryExpr{op}};
}
Expr Expr::makePrefix(const Token &op) {
  return Expr{op.sourceLocation, PrefixExpr{op}};
}
Expr Expr::makeInt(const Token &op, int num) {
  return Expr{op.sourceLocation, IntExpr{num}};
}
