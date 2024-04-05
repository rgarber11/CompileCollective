// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef INCLUDE_SRC_EXPR_H_
#define INCLUDE_SRC_EXPR_H_

#include <iostream>
#include <memory>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include "common.h"
#include "token.h"
#include "types.h"
template <typename T>
struct Visitor;
struct Expr;
struct Stmt;
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
struct CharExpr {
  char c;
  explicit CharExpr(char c) : c(c) {}
  CharExpr(const CharExpr& charExpr) = default;
  CharExpr(CharExpr&& charExpr) noexcept : c(charExpr.c) {}
};
struct StringExpr {
  std::string str;
  explicit StringExpr(const std::string_view str) : str(str) {}
  StringExpr(const StringExpr& stringExpr) = default;
  StringExpr(StringExpr&& stringExpr) noexcept : str(stringExpr.str) {}
};
struct ImplicitTypeConvExpr {
  std::shared_ptr<Type> from;  // Definitely won't stay TokenType
  std::shared_ptr<Type> to;
  std::unique_ptr<Expr> expr;
  explicit ImplicitTypeConvExpr(const Type& from, const Type& to);
  ImplicitTypeConvExpr(const ImplicitTypeConvExpr& implicitTypeConvExpr);
  ImplicitTypeConvExpr(ImplicitTypeConvExpr&& implicitTypeConvExpr) noexcept;
  ~ImplicitTypeConvExpr() = default;
};
struct FunctionExpr {
  std::unique_ptr<Expr> functionAccess;
  std::vector<Expr> parameters;
  FunctionExpr(const FunctionExpr& functionExpr);
  FunctionExpr(FunctionExpr&& functionExpr) noexcept;
  ~FunctionExpr() = default;
};
struct LiteralExpr {
  std::string name;
  LiteralExpr(const std::string_view name) : name(name){};
};
struct RangeExpr {
  bool inclusive;
  std::unique_ptr<Expr> from;
  std::unique_ptr<Expr> to;
  RangeExpr(const RangeExpr& range);
  RangeExpr(RangeExpr&& range) noexcept;
};
struct ForConditionExpr {
  RangeExpr range;
  LiteralExpr var;
  ForConditionExpr() = default;
  ForConditionExpr(const RangeExpr& range, const LiteralExpr& var)
      : range(range), var(var){};
  ForConditionExpr(const ForConditionExpr& for_condition_expr) = default;
  ForConditionExpr(ForConditionExpr&& for_condition_expr) noexcept = default;
};
struct CaseExpr {
  std::shared_ptr<Type> type;
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Expr> body;
  CaseExpr(const CaseExpr& caseExpr);
  CaseExpr(CaseExpr&& caseExpr) noexcept;
  ~CaseExpr() = default;
};
struct CaseType {
  std::shared_ptr<Type> type;
  std::shared_ptr<Type> condType;
  std::unique_ptr<Expr> body;
  CaseType(const CaseType& caseType);
  CaseType(CaseType&& caseType) noexcept;
  ~CaseType() = default;
};
struct MatchExpr {
  std::unique_ptr<Expr> cond;
  std::variant<std::vector<CaseExpr>, std::vector<CaseType>> cases;
  bool isTypeMatch() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>,
                                std::vector<CaseType>>;
        },
        cases);
  }
  MatchExpr(const MatchExpr& matchExpr);
  MatchExpr(MatchExpr&& matchExpr) noexcept;
  ~MatchExpr() = default;
};
struct IfExpr {
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Expr> thenExpr;
  std::unique_ptr<Expr> elseExpr;
  IfExpr(const IfExpr& ifExpr);
  IfExpr(IfExpr&& ifExpr) noexcept;
  ~IfExpr() = default;
};
struct BlockExpr {
  bool returns;
  bool yields;
  std::vector<Stmt> stmts;
};
struct ForExpr {
  std::unique_ptr<ForConditionExpr> expr;
  std::unique_ptr<Expr> body;
  ForExpr(const ForExpr& forExpr);
  ForExpr(ForExpr&& forExpr) noexcept;
  ~ForExpr() = default;
};
struct WhileExpr {
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Expr> body;
  WhileExpr(const WhileExpr& whileExpr);
  WhileExpr(WhileExpr&& whileExpr) noexcept;
  ~WhileExpr() = default;
};
struct GetExpr {
  std::unique_ptr<Expr> expr;
  LiteralExpr name;
  GetExpr() : expr(nullptr), name(""){};
  GetExpr(Expr expr, LiteralExpr name);
  GetExpr(const GetExpr& getExpr);
  GetExpr(GetExpr&& getExpr) noexcept;
  ~GetExpr() = default;
};
struct CallExpr {
  std::unique_ptr<Expr> expr;
  std::vector<Expr> params;
  CallExpr() = default;
  CallExpr(const CallExpr& callExpr);
  CallExpr(CallExpr&& callExpr) noexcept;
  ~CallExpr() = default;
};
using InnerExpr =
    std::variant<BinaryExpr, PrefixExpr, IntExpr, FloatExpr, CharExpr,
                 StringExpr, LiteralExpr, ForConditionExpr, RangeExpr,
                 FunctionExpr, ImplicitTypeConvExpr, MatchExpr, IfExpr,
                 BlockExpr, ForExpr, WhileExpr, GetExpr, CallExpr>;
struct Expr {
  const SourceLocation sourceLocation;
  std::shared_ptr<Type> type;
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
          } else if (std::is_same_v<T, CharExpr>) {
            return visitor->visitCharExpr(this);
          } else if (std::is_same_v<T, StringExpr>) {
            return visitor->visitStringExpr(this);
          } else if (std::is_same_v<T, LiteralExpr>) {
            return visitor->visitLiteralExpr(this);
          } else if (std::is_same_v<T, ForConditionExpr>) {
            return visitor->visitForConditionExpr(this);
          } else if (std::is_same_v<T, RangeExpr>) {
            return visitor->visitRangeExpr(this);
          } else if (std::is_same_v<T, FunctionExpr>) {
            return visitor->visitFunctionExpr(this);
          } else if (std::is_same_v<T, ImplicitTypeConvExpr>) {
            return visitor->visitImplicitTypeConvExpr(this);
          } else if (std::is_same_v<T, MatchExpr>) {
            return visitor->visitMatchExpr(this);
          } else if (std::is_same_v<T, IfExpr>) {
            return visitor->visitIfExpr(this);
          } else if (std::is_same_v<T, BlockExpr>) {
            return visitor->visitBlockExpr(this);
          } else if (std::is_same_v<T, ForExpr>) {
            return visitor->visitForExpr(this);
          } else if (std::is_same_v<T, WhileExpr>) {
            return visitor->visitWhileExpr(this);
          } else if (std::is_same_v<T, GetExpr>) {
            return visitor->visitGetExpr(this);
          } else if (std::is_same_v<T, CallExpr>) {
            return visitor->visitCallExpr(this);
          }
        },
        innerExpr);
  }
  [[nodiscard]] std::unique_ptr<Expr> clone() const {
    return std::make_unique<Expr>(sourceLocation, type, innerExpr);
  }
  static Expr makeBinary(const Token& op, const Type& token_type);
  static Expr makePrefix(const Token& op, const Type& token_type);
  static Expr makeInt(const Token& op, int num);
  static Expr makeFloat(const Token& op, double num);
  static Expr makeImplicitTypeConv(const SourceLocation& source_location,
                                   const Type& from, const Type& to);
  [[nodiscard]] BinaryExpr* getBinaryExpr() {
    return &std::get<BinaryExpr>(innerExpr);
  }
  [[nodiscard]] PrefixExpr* getPrefixExpr() {
    return &std::get<PrefixExpr>(innerExpr);
  }
  [[nodiscard]] IntExpr* getIntExpr() { return &std::get<IntExpr>(innerExpr); }
  [[nodiscard]] FloatExpr* getFloatExpr() {
    return &std::get<FloatExpr>(innerExpr);
  }
  [[nodiscard]] CharExpr* getCharExpr() {
    return &std::get<CharExpr>(innerExpr);
  }

  [[nodiscard]] StringExpr* getStringExpr() {
    return &std::get<StringExpr>(innerExpr);
  }
  [[nodiscard]] LiteralExpr* getLiteralExpr() {
    return &std::get<LiteralExpr>(innerExpr);
  }
  [[nodiscard]] ForConditionExpr* getForConditionExpr() {
    return &std::get<ForConditionExpr>(innerExpr);
  }
  [[nodiscard]] RangeExpr* getRangeExpr() {
    return &std::get<RangeExpr>(innerExpr);
  }

  [[nodiscard]] FunctionExpr* getFunctionExpr() {
    return &std::get<FunctionExpr>(innerExpr);
  }
  [[nodiscard]] ImplicitTypeConvExpr* getImplicitTypeConvExpr() {
    return &std::get<ImplicitTypeConvExpr>(innerExpr);
  }

  [[nodiscard]] MatchExpr* getMatchExpr() {
    return &std::get<MatchExpr>(innerExpr);
  }
  [[nodiscard]] IfExpr* getIfExpr() { return &std::get<IfExpr>(innerExpr); }
  [[nodiscard]] BlockExpr* getBlockExpr() {
    return &std::get<BlockExpr>(innerExpr);
  }
  [[nodiscard]] ForExpr* getForExpr() { return &std::get<ForExpr>(innerExpr); }
  [[nodiscard]] WhileExpr* getWhileExpr() {
    return &std::get<WhileExpr>(innerExpr);
  }

  [[nodiscard]] GetExpr* getGetExpr() { return &std::get<GetExpr>(innerExpr); }
  [[nodiscard]] CallExpr* getCallExpr() {
    return &std::get<CallExpr>(innerExpr);
  }
  [[nodiscard]] int getInt() const { return std::get<IntExpr>(innerExpr).val; }
  [[nodiscard]] double getFloat() const {
    return std::get<FloatExpr>(innerExpr).val;
  }
  [[nodiscard]] bool isBinaryExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, BinaryExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isPrefixExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, PrefixExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isIntExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, IntExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isFloatExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, FloatExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isCharExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, CharExpr>;
        },
        innerExpr);
  }

  [[nodiscard]] bool isStringExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, StringExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isLiteralExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, LiteralExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isForConditionExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, ForConditionExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isRangeExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, RangeExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isFunctionExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, FunctionExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isImplicitTypeConvExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>,
                                ImplicitTypeConvExpr>;
        },
        innerExpr);
  }

  [[nodiscard]] bool isMatchExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, MatchExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isIfExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, IfExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isBlockExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, BlockExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isForExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, ForExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isWhileExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, WhileExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isGetExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, GetExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isCallExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, CallExpr>;
        },
        innerExpr);
  }
  ~Expr() = default;
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
  virtual T visitBinaryExpr(Expr* binaryExpr) = 0;
  virtual T visitPrefixExpr(Expr* prefixExpr) = 0;
  virtual T visitIntExpr(Expr* intExpr) = 0;
  virtual T visitFloatExpr(Expr* floatExpr) = 0;
  virtual T visitCharExpr(Expr* charExpr) = 0;
  virtual T visitStringExpr(Expr* stringExpr) = 0;
  virtual T visitLiteralExpr(Expr* literalExpr) = 0;
  virtual T visitForConditionExpr(Expr* forConditionExpr) = 0;
  virtual T visitRangeExpr(Expr* rangeExpr) = 0;
  virtual T visitFunctionExpr(Expr* functionExpr) = 0;
  virtual T visitImplicitTypeConvExpr(Expr* implicitTypeConvExpr) = 0;
  virtual T visitMatchExpr(Expr* matchExpr) = 0;
  virtual T visitIfExpr(Expr* ifExpr) = 0;
  virtual T visitBlockExpr(Expr* blockExpr) = 0;
  virtual T visitForExpr(Expr* forExpr) = 0;
  virtual T visitWhileExpr(Expr* whileExpr) = 0;
};
#endif  // INCLUDE_SRC_EXPR_H_
