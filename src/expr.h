// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef INCLUDE_SRC_EXPR_H_
#define INCLUDE_SRC_EXPR_H_

#include <iostream>
#include <memory>
#include <type_traits>
#include <variant>

#include "common.h"
#include "token.h"
template <typename T>
struct Visitor;
struct Expr;
struct BinaryExpr {
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;
  TOKEN_TYPE op;
  explicit BinaryExpr(const Token& opToken);
  BinaryExpr(const BinaryExpr& binaryExpr);
  BinaryExpr(BinaryExpr&& binaryExpr) noexcept;
  ~BinaryExpr() = default;
};
struct PrefixExpr {
  std::unique_ptr<Expr> expr;
  TOKEN_TYPE op;
  explicit PrefixExpr(const Token& opToken);
  PrefixExpr(const PrefixExpr& prefixExpr);
  PrefixExpr(PrefixExpr&& prefixExpr) noexcept;
  ~PrefixExpr() = default;
};
struct IntExpr {
  int val;
  explicit IntExpr(int val) : val(val) {}
  IntExpr(const IntExpr& intExpr) = default;
  IntExpr(IntExpr&& intExpr) noexcept : val(intExpr.val) {}
};
using InnerExpr = std::variant<BinaryExpr, PrefixExpr, IntExpr>;
struct Expr {
  const SourceLocation sourceLocation;
  InnerExpr innerExpr;
  template <typename R>
  R accept(Visitor<R>* visitor) {
    return std::visit(
        [visitor, this](auto&& arg) -> R {
          using T = std::decay_t<decltype(arg)>;
          if (std::is_same_v<T, BinaryExpr>) {
            return visitor->visitBinaryExpr(this);
          } else if (std::is_same_v<T, PrefixExpr>) {
            return visitor->visitPrefixExpr(this);
          } else {
            return visitor->visitIntExpr(this);
          }
        },
        innerExpr);
  }
  [[nodiscard]] std::unique_ptr<Expr> clone() const {
    return std::make_unique<Expr>(sourceLocation, innerExpr);
  }
  static Expr makeBinary(const Token& op);
  static Expr makePrefix(const Token& op);
  static Expr makeInt(const Token& op, int num);
  [[nodiscard]] BinaryExpr* getBinary() {
    return &std::get<BinaryExpr>(innerExpr);
  }
  [[nodiscard]] PrefixExpr* getPrefix() {
    return &std::get<PrefixExpr>(innerExpr);
  }
  [[maybe_unused]] [[nodiscard]] IntExpr* getIntExpr() {
    return &std::get<IntExpr>(innerExpr);
  }
  [[nodiscard]] int getInt() const { return std::get<IntExpr>(innerExpr).val; }
  [[maybe_unused]] [[nodiscard]] bool isBinary() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, BinaryExpr>;
        },
        innerExpr);
  }
  [[maybe_unused]] [[nodiscard]] bool isPrefix() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, PrefixExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isInt() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, IntExpr>;
        },
        innerExpr);
  }
};

template <typename T>
struct Visitor {
  T visit(Expr* expr) {
    enterVisitor();
    if constexpr (std::is_same_v<T, void>) {
      expr->accept(this);
      exitVisitor();
    } else {
      T ans = expr->accept(this);
      exitVisitor();
      return ans;
    }
  }
  T _visit(Expr* expr) { return expr->accept(this); }
  virtual void enterVisitor() = 0;
  virtual void exitVisitor() = 0;
  virtual T visitBinaryExpr(Expr* expr) = 0;
  virtual T visitPrefixExpr(Expr* expr) = 0;
  virtual T visitIntExpr(Expr* expr) = 0;
};
#endif  // INCLUDE_SRC_EXPR_H_
