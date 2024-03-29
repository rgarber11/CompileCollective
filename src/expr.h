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
struct FloatExpr {
  double val;
  explicit FloatExpr(double val) : val(val) {}
  FloatExpr(const FloatExpr& floatExpr) = default;
  FloatExpr(FloatExpr&& floatExpr) noexcept : val(floatExpr.val) {}
};
struct ImplicitTypeConvExpr {
  TOKEN_TYPE from;  // Definitely won't stay TokenType
  TOKEN_TYPE to;
  std::unique_ptr<Expr> expr;
  explicit ImplicitTypeConvExpr(const TOKEN_TYPE& from, const TOKEN_TYPE& to);
  ImplicitTypeConvExpr(const ImplicitTypeConvExpr& implicitTypeConvExpr);
  ImplicitTypeConvExpr(ImplicitTypeConvExpr&& implicitTypeConvExpr) noexcept;
  ~ImplicitTypeConvExpr() = default;
};
using InnerExpr = std::variant<BinaryExpr, PrefixExpr, IntExpr, FloatExpr,
                               ImplicitTypeConvExpr>;
struct Expr {
  const SourceLocation sourceLocation;
  TOKEN_TYPE type;
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
          } else if (std::is_same_v<T, IntExpr>) {
            return visitor->visitIntExpr(this);
          } else if (std::is_same_v<T, FloatExpr>) {
            return visitor->visitFloatExpr(this);
          } else {
            return visitor->visitImplicitTypeConvExpr(this);
          }
        },
        innerExpr);
  }
  [[nodiscard]] std::unique_ptr<Expr> clone() const {
    return std::make_unique<Expr>(sourceLocation, type, innerExpr);
  }
  static Expr makeBinary(const Token& op, const TOKEN_TYPE& token_type);
  static Expr makePrefix(const Token& op, const TOKEN_TYPE& token_type);
  static Expr makeInt(const Token& op, int num);
  static Expr makeFloat(const Token& op, double num);
  static Expr makeImplicitTypeConv(const SourceLocation& source_location,
                                   const TOKEN_TYPE& from,
                                   const TOKEN_TYPE& to);
  [[nodiscard]] BinaryExpr* getBinary() {
    return &std::get<BinaryExpr>(innerExpr);
  }
  [[nodiscard]] PrefixExpr* getPrefix() {
    return &std::get<PrefixExpr>(innerExpr);
  }
  [[maybe_unused]] [[nodiscard]] IntExpr* getIntExpr() {
    return &std::get<IntExpr>(innerExpr);
  }
  [[maybe_unused]] [[nodiscard]] FloatExpr* getFloatExpr() {
    return &std::get<FloatExpr>(innerExpr);
  }
  [[maybe_unused]] [[nodiscard]] ImplicitTypeConvExpr*
  getImplicitTypeConvExpr() {
    return &std::get<ImplicitTypeConvExpr>(innerExpr);
  }
  [[nodiscard]] int getInt() const { return std::get<IntExpr>(innerExpr).val; }
  [[nodiscard]] double getFloat() const {
    return std::get<FloatExpr>(innerExpr).val;
  }
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
  [[nodiscard]] bool isFloat() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, FloatExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isImplicitTypeConvExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>,
                                ImplicitTypeConvExpr>;
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
  virtual T visitFloatExpr(Expr* expr) = 0;
  virtual T visitImplicitTypeConvExpr(Expr* expr) = 0;
};
#endif  // INCLUDE_SRC_EXPR_H_
