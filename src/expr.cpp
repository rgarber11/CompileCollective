// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#include "expr.h"

#include <memory>
#include "environment.h"
#include "stmt.h"
#include "token.h"
#include "types.h"

// Copy and contructor operations
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
TypeConvExpr::TypeConvExpr(bool implicit, std::shared_ptr<Type> from,
                           std::shared_ptr<Type> to)
    : implicit(implicit), from(from), to(to) {}
TypeConvExpr::TypeConvExpr(std::shared_ptr<Type> from, std::shared_ptr<Type> to)
    : from(from), to(to) {}
TypeConvExpr::TypeConvExpr(const TypeConvExpr &implicitTypeConvExpr)
    : from(implicitTypeConvExpr.from),
      to(implicitTypeConvExpr.to),
      expr(implicitTypeConvExpr.expr ? implicitTypeConvExpr.expr->clone()
                                     : nullptr) {}
TypeConvExpr::TypeConvExpr(TypeConvExpr &&implicitTypeConvExpr) noexcept
    : from(implicitTypeConvExpr.from),
      to(implicitTypeConvExpr.to),
      expr(implicitTypeConvExpr.expr ? std::move(implicitTypeConvExpr.expr)
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
    : env(nullptr), body(forExpr.body ? forExpr.body->clone() : nullptr) {}
ForExpr::ForExpr(ForExpr &&forExpr) noexcept
    : env(forExpr.env ? std::move(forExpr.env) : nullptr),
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
  // Copy all parameters
  for (const auto &i : callExpr.params) {
    params.emplace_back(i->clone());
  }
}
CallExpr::CallExpr(CallExpr &&callExpr) noexcept
    : expr(callExpr.expr ? callExpr.expr->clone() : nullptr) {
  // Copy all parameters
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
// Make expressions of various types
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
Expr Expr::makeTypeConv(const SourceLocation &source_location,
                        std::shared_ptr<Type> from, std::shared_ptr<Type> to) {
  return Expr{source_location, to, TypeConvExpr{false, from, to}};
}
// Destructors (default)
ForExpr::~ForExpr() = default;
BlockExpr::~BlockExpr() = default;
FunctionExpr::~FunctionExpr() = default;
// Copy and contructor operations
BlockExpr::BlockExpr(const BlockExpr &blockExpr) : returns(blockExpr.returns), yields(blockExpr.yields), env(blockExpr.env ? blockExpr.env->clone() : nullptr){
  // Copy all statements
  for(auto& stmt : blockExpr.stmts) {
    stmts.emplace_back(std::move(stmt->clone()));
  }
}
BlockExpr::BlockExpr(BlockExpr &&blockExpr) noexcept: returns(blockExpr.returns), yields(blockExpr.yields), env(std::move(blockExpr.env)){
  // Copy all statements
  for(auto& stmt : blockExpr.stmts) {
    stmts.push_back(std::move(stmt));
  }
}
// Copy, destructor, and constructor operations (default or no implementations)
Expr::Expr(const Expr& expr) = default;
Expr::Expr(Expr&& expr) noexcept = default;
Expr::Expr(const SourceLocation& source_location, std::shared_ptr<Type> type, const InnerExpr& inner_expr) : sourceLocation(source_location), type(std::move(type)), innerExpr(inner_expr) {}
Expr::~Expr() = default;
FunctionExpr::FunctionExpr(const FunctionExpr& functionExpr) : arity(functionExpr.arity), parameters(functionExpr.parameters->clone()), returnType(functionExpr.returnType), action(functionExpr.action->clone()) {}
FunctionExpr::FunctionExpr(FunctionExpr&& functionExpr) noexcept = default;
IfExpr::~IfExpr() = default;
CaseExpr::CaseExpr(const CaseExpr& caseExpr) : type(caseExpr.type), cond(caseExpr.cond), body(caseExpr.body->clone()) {}
CaseExpr::CaseExpr(CaseExpr&& caseExpr) noexcept : type(std::move(caseExpr.type)), cond(std::move(caseExpr.cond)), body(std::move(caseExpr.body)) {}
CaseExpr::~CaseExpr() = default;
TypeConvExpr::~TypeConvExpr() = default;
BinaryExpr::~BinaryExpr() = default;
PrefixExpr::~PrefixExpr() = default;
GetExpr::~GetExpr() = default;
CallExpr::~CallExpr() = default;
WhileExpr::~WhileExpr() = default;
MatchExpr::~MatchExpr() = default;
ForConditionExpr::~ForConditionExpr() = default;
