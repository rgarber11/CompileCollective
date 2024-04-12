// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#include "expr.h"

#include <memory>

#include "token.h"
#include "types.h"

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
ImplicitTypeConvExpr::ImplicitTypeConvExpr(const Type &from, const Type &to)
    : from(from.clone()), to(to.clone()) {}
ImplicitTypeConvExpr::ImplicitTypeConvExpr(
    const ImplicitTypeConvExpr &implicitTypeConvExpr)
    : from(implicitTypeConvExpr.from),
      to(implicitTypeConvExpr.to),
      expr(implicitTypeConvExpr.expr ? implicitTypeConvExpr.expr->clone()
                                     : nullptr) {}
ImplicitTypeConvExpr::ImplicitTypeConvExpr(
    ImplicitTypeConvExpr &&implicitTypeConvExpr) noexcept
    : from(implicitTypeConvExpr.from),
      to(implicitTypeConvExpr.to),
      expr(implicitTypeConvExpr.expr ? implicitTypeConvExpr.expr->clone()
                                     : nullptr) {}
MatchExpr::MatchExpr(const MatchExpr &matchExpr)
    : cond(matchExpr.cond ? matchExpr.cond->clone() : nullptr),
      cases(matchExpr.cases) {}
MatchExpr::MatchExpr(MatchExpr &&matchExpr) noexcept
    : cond(matchExpr.cond ? matchExpr.cond->clone() : nullptr),
      cases(matchExpr.cases) {}
IfExpr::IfExpr(const IfExpr &ifExpr)
    : cond(ifExpr.cond ? ifExpr.cond->clone() : nullptr),
      thenExpr(ifExpr.thenExpr ? ifExpr.thenExpr->clone() : nullptr),
      elseExpr(ifExpr.elseExpr ? ifExpr.elseExpr->clone() : nullptr) {}
IfExpr::IfExpr(IfExpr &&ifExpr) noexcept
    : cond(ifExpr.cond ? ifExpr.cond->clone() : nullptr),
      thenExpr(ifExpr.thenExpr ? ifExpr.thenExpr->clone() : nullptr),
      elseExpr(ifExpr.elseExpr ? ifExpr.elseExpr->clone() : nullptr) {}
ForExpr::ForExpr(const ForExpr &forExpr)
    : expr(forExpr.expr),
      body(forExpr.body ? forExpr.body->clone() : nullptr) {}
ForExpr::ForExpr(ForExpr &&forExpr) noexcept
    : expr(forExpr.expr),
      body(forExpr.body ? forExpr.body->clone() : nullptr) {}
WhileExpr::WhileExpr(const WhileExpr &whileExpr)
    : cond(whileExpr.cond ? whileExpr.cond->clone() : nullptr),
      body(whileExpr.body ? whileExpr.body->clone() : nullptr) {}
WhileExpr::WhileExpr(WhileExpr &&whileExpr) noexcept
    : cond(whileExpr.cond ? whileExpr.cond->clone() : nullptr),
      body(whileExpr.body ? whileExpr.body->clone() : nullptr) {}
GetExpr::GetExpr(const GetExpr &getExpr)
    : expr(getExpr.expr ? getExpr.expr->clone() : nullptr),
      name(getExpr.name) {}
GetExpr::GetExpr(GetExpr &&getExpr) noexcept
    : expr(getExpr.expr ? getExpr.expr->clone() : nullptr),
      name(getExpr.name) {}
GetExpr::GetExpr(Expr expr, LiteralExpr name)
    : expr(expr.clone()), name(name){};
CallExpr::CallExpr(const CallExpr &callExpr)
    : expr(callExpr.expr ? callExpr.expr->clone() : nullptr) {
  for (const auto &i : callExpr.params) {
    params.emplace_back(i->clone());
  }
}
CallExpr::CallExpr(CallExpr &&callExpr) noexcept
    : expr(callExpr.expr ? callExpr.expr->clone() : nullptr) {
  for (const auto &i : callExpr.params) {
    params.emplace_back(i->clone());
  }
}
ForConditionExpr::ForConditionExpr(const ForConditionExpr &for_condition_expr)
    : expr(for_condition_expr.expr ? for_condition_expr.expr->clone()
                                   : nullptr),
      var(for_condition_expr.var) {}
ForConditionExpr::ForConditionExpr(
    ForConditionExpr &&for_condition_expr) noexcept
    : expr(for_condition_expr.expr ? for_condition_expr.expr->clone()
                                   : nullptr),
      var(for_condition_expr.var) {}
Expr Expr::makeBinary(const Token &op, std::shared_ptr<Type> type) {
  return Expr{op.sourceLocation, type, BinaryExpr{op}};
}
Expr Expr::makePrefix(const Token &op, std::shared_ptr<Type> type) {
  return Expr{op.sourceLocation, type, PrefixExpr{op}};
}
Expr Expr::makeInt(const Token &op, std::shared_ptr<Type> type, int num) {
  return Expr{op.sourceLocation, type, IntExpr{num}};
}
Expr Expr::makeFloat(const Token &op, std::shared_ptr<Type> type, double num) {
  return Expr{op.sourceLocation, type, FloatExpr{num}};
}
Expr Expr::makeImplicitTypeConv(const SourceLocation &source_location,
                                const Type &from, const Type &to) {
  return Expr{source_location, to.clone(), ImplicitTypeConvExpr{from, to}};
}
