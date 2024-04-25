// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#include "expr.h"

#include <algorithm>
#include <memory>

#include "environment.h"
#include "stmt.h"
#include "token.h"
#include "types.h"

BinaryExpr::BinaryExpr(const Token& opToken) : op(opToken.type) {}
// Copy and contructor operations
BinaryExpr::BinaryExpr(const BinaryExpr& binaryExpr)
    : left(binaryExpr.left ? binaryExpr.left->clone() : nullptr),
      right(binaryExpr.right ? binaryExpr.right->clone() : nullptr),
      op(binaryExpr.op) {}
BinaryExpr::BinaryExpr(BinaryExpr&& binaryExpr) noexcept
    : left(binaryExpr.left ? binaryExpr.left->clone() : nullptr),
      right(binaryExpr.right ? binaryExpr.right->clone() : nullptr),
      op(binaryExpr.op) {}
PrefixExpr::PrefixExpr(const PrefixExpr& prefixExpr)
    : expr(prefixExpr.expr ? prefixExpr.expr->clone() : nullptr),
      op(prefixExpr.op) {}
PrefixExpr::PrefixExpr(const Token& opToken) : op(opToken.type) {}
PrefixExpr::PrefixExpr(PrefixExpr&& prefixExpr) noexcept
    : expr(prefixExpr.expr ? prefixExpr.expr->clone() : nullptr),
      op(prefixExpr.op) {}
TypeConvExpr::TypeConvExpr(bool implicit, std::shared_ptr<Type> from,
                           std::shared_ptr<Type> to)
    : implicit(implicit), from(from), to(to) {}
TypeConvExpr::TypeConvExpr(std::shared_ptr<Type> from, std::shared_ptr<Type> to)
    : from(from), to(to) {}
TypeConvExpr::TypeConvExpr(const TypeConvExpr& implicitTypeConvExpr)
    : implicit(implicitTypeConvExpr.implicit),
      from(implicitTypeConvExpr.from),
      to(implicitTypeConvExpr.to),
      expr(implicitTypeConvExpr.expr ? implicitTypeConvExpr.expr->clone()
                                     : nullptr) {}
TypeConvExpr::TypeConvExpr(TypeConvExpr&& implicitTypeConvExpr) noexcept
    : implicit(implicitTypeConvExpr.implicit),
      from(implicitTypeConvExpr.from),
      to(implicitTypeConvExpr.to),
      expr(implicitTypeConvExpr.expr ? std::move(implicitTypeConvExpr.expr)
                                     : nullptr) {}
MatchExpr::MatchExpr(const MatchExpr& matchExpr)
    : cond(matchExpr.cond ? matchExpr.cond->clone() : nullptr),
      cases(matchExpr.cases) {}
MatchExpr::MatchExpr(MatchExpr&& matchExpr) noexcept
    : cond(matchExpr.cond ? matchExpr.cond->clone() : nullptr),
      cases(matchExpr.cases) {}
IfExpr::IfExpr(const IfExpr& ifExpr)
    : cond(ifExpr.cond ? ifExpr.cond->clone() : nullptr),
      thenExpr(ifExpr.thenExpr ? ifExpr.thenExpr->clone() : nullptr),
      elseExpr(ifExpr.elseExpr ? ifExpr.elseExpr->clone() : nullptr) {}
IfExpr::IfExpr(IfExpr&& ifExpr) noexcept
    : cond(ifExpr.cond ? ifExpr.cond->clone() : nullptr),
      thenExpr(ifExpr.thenExpr ? ifExpr.thenExpr->clone() : nullptr),
      elseExpr(ifExpr.elseExpr ? ifExpr.elseExpr->clone() : nullptr) {}
ForExpr::ForExpr(const ForExpr& forExpr)
    : env(nullptr), body(forExpr.body ? forExpr.body->clone() : nullptr) {}
ForExpr::ForExpr(ForExpr&& forExpr) noexcept
    : env(forExpr.env ? std::move(forExpr.env) : nullptr),
      body(forExpr.body ? forExpr.body->clone() : nullptr) {}
WhileExpr::WhileExpr(const WhileExpr& whileExpr)
    : cond(whileExpr.cond ? whileExpr.cond->clone() : nullptr),
      body(whileExpr.body ? whileExpr.body->clone() : nullptr) {}
WhileExpr::WhileExpr(WhileExpr&& whileExpr) noexcept
    : cond(whileExpr.cond ? whileExpr.cond->clone() : nullptr),
      body(whileExpr.body ? whileExpr.body->clone() : nullptr) {}
GetExpr::GetExpr(const GetExpr& getExpr)
    : expr(getExpr.expr ? getExpr.expr->clone() : nullptr),
      name(getExpr.name) {}
GetExpr::GetExpr(GetExpr&& getExpr) noexcept
    : expr(getExpr.expr ? getExpr.expr->clone() : nullptr),
      name(getExpr.name) {}
GetExpr::GetExpr(Expr expr, LiteralExpr name)
    : expr(expr.clone()), name(name) {}
CallExpr::CallExpr(const CallExpr& callExpr)
    : expr(callExpr.expr ? callExpr.expr->clone() : nullptr) {
  // Copy all parameters
  for (const auto& i : callExpr.params) {
    params.emplace_back(i->clone());
  }
}
CallExpr::CallExpr(CallExpr&& callExpr) noexcept
    : expr(callExpr.expr ? callExpr.expr->clone() : nullptr) {
  // Copy all parameters
  for (const auto& i : callExpr.params) {
    params.emplace_back(i->clone());
  }
}
ForConditionExpr::ForConditionExpr(const ForConditionExpr& for_condition_expr)
    : expr(for_condition_expr.expr ? for_condition_expr.expr->clone()
                                   : nullptr),
      var(for_condition_expr.var) {}
ForConditionExpr::ForConditionExpr(
    ForConditionExpr&& for_condition_expr) noexcept
    : expr(for_condition_expr.expr ? for_condition_expr.expr->clone()
                                   : nullptr),
      var(for_condition_expr.var) {}
// Make expressions of various types
Expr Expr::makeBinary(const Token& op, std::shared_ptr<Type> type) {
  return Expr{op.sourceLocation, type, BinaryExpr{op}};
}
Expr Expr::makePrefix(const Token& op, std::shared_ptr<Type> type) {
  return Expr{op.sourceLocation, type, PrefixExpr{op}};
}
Expr Expr::makeInt(const Token& op, std::shared_ptr<Type> type, int num) {
  return Expr{op.sourceLocation, type, IntExpr{num}};
}
Expr Expr::makeFloat(const Token& op, std::shared_ptr<Type> type, double num) {
  return Expr{op.sourceLocation, type, FloatExpr{num}};
}
Expr Expr::makeTypeConv(const SourceLocation& source_location,
                        std::shared_ptr<Type> from, std::shared_ptr<Type> to) {
  return Expr{source_location, to, TypeConvExpr{false, from, to}};
}
// Destructors (default)
ForExpr::~ForExpr() = default;
BlockExpr::~BlockExpr() = default;
FunctionExpr::~FunctionExpr() = default;
// Copy and contructor operations
BlockExpr::BlockExpr(const BlockExpr& blockExpr)
    : returns(blockExpr.returns),
      yields(blockExpr.yields),
      env(blockExpr.env ? blockExpr.env->clone() : nullptr) {
  // Copy all statements
  for (auto& stmt : blockExpr.stmts) {
    stmts.emplace_back(std::move(stmt->clone()));
  }
}
BlockExpr::BlockExpr(BlockExpr&& blockExpr) noexcept
    : returns(blockExpr.returns),
      yields(blockExpr.yields),
      env(std::move(blockExpr.env)) {
  // Copy all statements
  for (auto& stmt : blockExpr.stmts) {
    stmts.push_back(std::move(stmt));
  }
}
// Copy, destructor, and constructor operations (default or no implementations)
Expr::Expr(const Expr& expr) = default;
Expr::Expr(Expr&& expr) noexcept = default;
Expr::Expr(const SourceLocation& source_location, std::shared_ptr<Type> type,
           const InnerExpr& inner_expr)
    : sourceLocation(source_location),
      type(std::move(type)),
      innerExpr(std::move(inner_expr)) {}
Expr::~Expr() = default;
FunctionExpr::FunctionExpr(const FunctionExpr& functionExpr)
    : arity(functionExpr.arity),
      name(functionExpr.name),
      parameters(functionExpr.parameters ? functionExpr.parameters->clone() : nullptr),
      returnType(functionExpr.returnType),
      action(functionExpr.action ? functionExpr.action->clone() : nullptr) {}
FunctionExpr::FunctionExpr(FunctionExpr&& functionExpr) noexcept : arity(functionExpr.arity), name(std::move(functionExpr.name)), parameters(std::move(functionExpr.parameters)), returnType(std::move(functionExpr.returnType)), action(std::move(functionExpr.action)) {}
IfExpr::~IfExpr() = default;
CaseExpr::CaseExpr(const CaseExpr& caseExpr)
    : type(caseExpr.type), body(caseExpr.body ? caseExpr.body->clone() : nullptr) {
  if (caseExpr.isExprCond()) {
    cond = caseExpr.getExpr()->clone();
  } else if (caseExpr.isTypeCond()) {
    cond = caseExpr.getTypeCase();
  } else {
    cond = std::get<std::string>(caseExpr.cond);
  }
}
CaseExpr::CaseExpr(CaseExpr&& caseExpr) noexcept
    : type(std::move(caseExpr.type)),
      cond(std::move(caseExpr.cond)),
      body(std::move(caseExpr.body)) {}
CaseExpr::~CaseExpr() = default;
TypeConvExpr::~TypeConvExpr() = default;
BinaryExpr::~BinaryExpr() = default;
PrefixExpr::~PrefixExpr() = default;
GetExpr::~GetExpr() = default;
CallExpr::~CallExpr() = default;
WhileExpr::~WhileExpr() = default;
MatchExpr::~MatchExpr() = default;
ForConditionExpr::~ForConditionExpr() = default;
BinaryExpr& BinaryExpr::operator=(const BinaryExpr& other) {
  this->op = other.op;
  this->left = other.left ? other.left->clone() : nullptr;
  this->right = other.right ? other.right->clone() : nullptr;
  return *this;
}
BinaryExpr& BinaryExpr::operator=(BinaryExpr&& other) noexcept {
  this->op = std::move(other.op);
  this->left = std::move(other.left);
  this->right = std::move(other.right);
  return *this;
}
PrefixExpr& PrefixExpr::operator=(const PrefixExpr& other) {
  this->op = other.op;
  this->expr = other.expr ? other.expr->clone() : nullptr;
  return *this;
}
PrefixExpr& PrefixExpr::operator=(PrefixExpr&& other) noexcept {
  this->op = std::move(other.op);
  this->expr = std::move(other.expr);
  return *this;
}
TypeConvExpr& TypeConvExpr::operator=(const TypeConvExpr& other) {
  this->implicit = other.implicit;
  this->from = other.from;
  this->to = other.to;
  this->expr = other.expr ? other.expr->clone() : nullptr;
  return *this;
}
TypeConvExpr& TypeConvExpr::operator=(TypeConvExpr&& other) noexcept {
  this->implicit = other.implicit;
  this->from = std::move(other.from);
  this->to = std::move(other.to);
  this->expr = std::move(other.expr);
  return *this;
}

ForConditionExpr& ForConditionExpr::operator=(const ForConditionExpr& other) {
  this->expr = other.expr ? other.expr->clone() : nullptr;
  this->var = other.var;
  return *this;
}
ForConditionExpr& ForConditionExpr::operator=(
    ForConditionExpr&& other) noexcept {
  this->expr = std::move(other.expr);
  this->var = std::move(other.var);
  return *this;
}

CaseExpr& CaseExpr::operator=(const CaseExpr& other) {
  type = other.type;
  if (other.isExprCond()) {
    cond = other.getExpr()->clone();
  } else if (other.isTypeCond()) {
    cond = other.getTypeCase();
  } else {
    cond = std::get<std::string>(other.cond);
  }
  body = other.body->clone();
  return *this;
}
CaseExpr& CaseExpr::operator=(CaseExpr&& other) noexcept {
  type = std::move(other.type);
  cond = std::move(other.cond);
  body = std::move(other.body);
  return *this;
}

MatchExpr& MatchExpr::operator=(const MatchExpr& other) {
  cond = other.cond ? other.cond->clone() : nullptr;
  for (auto& cased : other.cases) {
    cases.emplace_back(cased);
  }
  return *this;
}
MatchExpr& MatchExpr::operator=(MatchExpr&& other) noexcept {
  cond = std::move(other.cond);
  for (auto& cased : other.cases) {
    cases.emplace_back(std::move(cased));
  }
  return *this;
}

IfExpr& IfExpr::operator=(const IfExpr& other) {
  cond = other.cond ? other.cond->clone() : nullptr;
  thenExpr = other.thenExpr ? other.thenExpr->clone() : nullptr;
  elseExpr = other.elseExpr ? other.elseExpr->clone() : nullptr;
  return *this;
}
IfExpr& IfExpr::operator=(IfExpr&& other) noexcept {
  cond = std::move(other.cond);
  thenExpr = std::move(other.thenExpr);
  elseExpr = std::move(other.elseExpr);
  return *this;
}

BlockExpr& BlockExpr::operator=(const BlockExpr& blockExpr) {
  returns = blockExpr.returns;
  yields = blockExpr.yields;
  for (auto& stmt : blockExpr.stmts) {
    stmts.emplace_back(stmt->clone());
  }
  env = blockExpr.env ? blockExpr.env->clone() : nullptr;
  return *this;
}
BlockExpr& BlockExpr::operator=(BlockExpr&& blockExpr) noexcept {
  returns = blockExpr.returns;
  yields = blockExpr.yields;
  for (auto& stmt : blockExpr.stmts) {
    stmts.emplace_back(std::move(stmt));
  }
  env = std::move(blockExpr.env);
  return *this;
}

ForExpr& ForExpr::operator=(const ForExpr& forExpr) {
  env = forExpr.env ? forExpr.env->clone() : nullptr;
  body = forExpr.body ? forExpr.body->clone() : nullptr;
  return *this;
}
ForExpr& ForExpr::operator=(ForExpr&& forExpr) noexcept {
  env = std::move(forExpr.env);
  body = std::move(forExpr.body);
  return *this;
}

WhileExpr& WhileExpr::operator=(const WhileExpr& whileExpr) {
  cond = whileExpr.cond ? whileExpr.cond->clone() : nullptr;
  body = whileExpr.body ? whileExpr.body->clone() : nullptr;
  return *this;
}
WhileExpr& WhileExpr::operator=(WhileExpr&& whileExpr) noexcept {
  cond = std::move(whileExpr.cond);
  body = std::move(whileExpr.body);
  return *this;
}

GetExpr& GetExpr::operator=(const GetExpr& getExpr) {
  expr = getExpr.expr ? getExpr.expr->clone() : nullptr;
  name = getExpr.name;
  return *this;
}
GetExpr& GetExpr::operator=(GetExpr&& getExpr) noexcept {
  expr = std::move(getExpr.expr);
  name = std::move(getExpr.name);
  return *this;
}

CallExpr& CallExpr::operator=(const CallExpr& callExpr) {
  expr = callExpr.expr ? callExpr.expr->clone() : nullptr;
  for (auto& param : callExpr.params) {
    params.emplace_back(param->clone());
  }
  return *this;
}
CallExpr& CallExpr::operator=(CallExpr&& callExpr) noexcept {
  expr = std::move(callExpr.expr);
  for (auto& param : callExpr.params) {
    params.emplace_back(std::move(param));
  }
  return *this;
}

FunctionExpr& FunctionExpr::operator=(const FunctionExpr& functionExpr) {
  if (this == &functionExpr) return *this;
  arity = functionExpr.arity;
  name = functionExpr.name;
  parameters = functionExpr.parameters ? functionExpr.parameters->clone() : nullptr;
  returnType = functionExpr.returnType;
  action = functionExpr.action ? functionExpr.action->clone() : nullptr;
  return *this;
}
FunctionExpr& FunctionExpr::operator=(FunctionExpr&& functionExpr) noexcept {
  arity = functionExpr.arity;
  name = std::move(functionExpr.name);
  parameters = std::move(functionExpr.parameters);
  returnType = std::move(functionExpr.returnType);
  action = std::move(functionExpr.action);
  return *this;
}
