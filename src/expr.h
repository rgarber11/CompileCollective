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
struct ExprVisitor;
struct Expr;
struct Stmt;
struct Environment;
// Binary expression - two expressions with an operation
struct BinaryExpr {
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;
  TOKEN_TYPE op;
  explicit BinaryExpr(const Token& opToken);
  BinaryExpr(const BinaryExpr& binaryExpr);
  BinaryExpr(BinaryExpr&& binaryExpr) noexcept;
  BinaryExpr& operator=(const BinaryExpr& other);
  BinaryExpr& operator=(BinaryExpr&& other) noexcept;
  ~BinaryExpr();
};
// Prefix expression - expression with an operation before it
struct PrefixExpr {
  std::unique_ptr<Expr> expr;
  TOKEN_TYPE op;
  explicit PrefixExpr(const Token& opToken);
  PrefixExpr(const PrefixExpr& prefixExpr);
  PrefixExpr(PrefixExpr&& prefixExpr) noexcept;
  PrefixExpr& operator=(const PrefixExpr& other);
  PrefixExpr& operator=(PrefixExpr&& other) noexcept;
  ~PrefixExpr();
};
// Int expression - one integer
struct IntExpr {
  int val;
};
// Bool expression - one boolean
struct BoolExpr {
  bool val;
};
// Float expression - one float
struct FloatExpr {
  double val;
};
// Char expression - one character
struct CharExpr {
  char c;
};
// String expression - one string
struct StringExpr {
  std::string str;
  explicit StringExpr(const std::string_view str) : str(str) {}
  StringExpr(const StringExpr& stringExpr) = default;
  StringExpr(StringExpr&& stringExpr) noexcept
      : str(std::move(stringExpr.str)) {}
  StringExpr& operator=(const StringExpr& other) = default;
  StringExpr& operator=(StringExpr&& other) = default;
  ~StringExpr() = default;
};
// Type conversion expression - implicit or explicit
struct TypeConvExpr {
  bool implicit;
  std::shared_ptr<Type> from;  // Definitely won't stay TokenType
  std::shared_ptr<Type> to;
  std::unique_ptr<Expr> expr;
  explicit TypeConvExpr(bool implicit, std::shared_ptr<Type> from,
                        std::shared_ptr<Type> to);
  explicit TypeConvExpr(std::shared_ptr<Type> from, std::shared_ptr<Type> to);
  TypeConvExpr(const TypeConvExpr& implicitTypeConvExpr);
  TypeConvExpr(TypeConvExpr&& implicitTypeConvExpr) noexcept;
  TypeConvExpr& operator=(const TypeConvExpr& other);
  TypeConvExpr& operator=(TypeConvExpr&& other) noexcept;
  ~TypeConvExpr();
};
// Literal expression - a string name
struct LiteralExpr {
  std::string name;
  LiteralExpr() = default;
  LiteralExpr(const std::string_view name) : name(name){};
  LiteralExpr(const LiteralExpr& literalExpr) = default;
  LiteralExpr(LiteralExpr&& literalExpr) noexcept = default;
  LiteralExpr& operator=(const LiteralExpr& other) = default;
  LiteralExpr& operator=(LiteralExpr&& other) noexcept = default;
  ~LiteralExpr() = default;
};
// For condition expression - expression and literal expression
struct ForConditionExpr {
  std::unique_ptr<Expr> expr;
  LiteralExpr var;
  ForConditionExpr() = default;
  ForConditionExpr(const ForConditionExpr& for_condition_expr);
  ForConditionExpr(ForConditionExpr&& for_condition_expr) noexcept;
  ForConditionExpr& operator=(const ForConditionExpr& other);
  ForConditionExpr& operator=(ForConditionExpr&& other) noexcept;
  ~ForConditionExpr();
};
// Case expression - type, condition, and body
struct CaseExpr {
  std::shared_ptr<Type> type;
  std::string cond;
  std::unique_ptr<Expr> body;
  CaseExpr() = default;
  CaseExpr(const CaseExpr& caseExpr);
  CaseExpr(CaseExpr&& caseExpr) noexcept;
  CaseExpr& operator=(const CaseExpr& other);
  CaseExpr& operator=(CaseExpr&& other) noexcept;
  ~CaseExpr();
};
// Match expression - condition and cases
struct MatchExpr {
  std::unique_ptr<Expr> cond;
  std::vector<CaseExpr> cases;
  MatchExpr() = default;
  MatchExpr(const MatchExpr& matchExpr);
  MatchExpr(MatchExpr&& matchExpr) noexcept;
  MatchExpr& operator=(const MatchExpr& other);
  MatchExpr& operator=(MatchExpr&& other) noexcept;
  ~MatchExpr();
};
// If expression - condition, if-then, if-else
struct IfExpr {
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Expr> thenExpr;
  std::unique_ptr<Expr> elseExpr;
  IfExpr() = default;
  IfExpr(const IfExpr& ifExpr);
  IfExpr(IfExpr&& ifExpr) noexcept;
  IfExpr& operator=(const IfExpr& other);
  IfExpr& operator=(IfExpr&& other) noexcept;
  ~IfExpr();
};
// Block expression - an environment (scope) with statements, and a possible
// return or yield
struct BlockExpr {
  bool returns;
  bool yields;
  std::vector<std::unique_ptr<Stmt>> stmts;
  std::unique_ptr<Environment> env;
  BlockExpr() = default;
  BlockExpr(const BlockExpr& blockExpr);
  BlockExpr(BlockExpr&& blockExpr) noexcept;
  BlockExpr& operator=(const BlockExpr& blockExpr);
  BlockExpr& operator=(BlockExpr&& blockExpr) noexcept;
  ~BlockExpr();
};
// For expression - an environment (scope) and a body
struct ForExpr {
  std::unique_ptr<Environment> env;
  std::unique_ptr<Expr> body;
  ForExpr() = default;
  ForExpr(const ForExpr& forExpr);
  ForExpr(ForExpr&& forExpr) noexcept;
  ForExpr& operator=(const ForExpr& forExpr);
  ForExpr& operator=(ForExpr&& forExpr) noexcept;
  ~ForExpr();
};
// While expression - a condition and a body
struct WhileExpr {
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Expr> body;
  WhileExpr() = default;
  WhileExpr(const WhileExpr& whileExpr);
  WhileExpr(WhileExpr&& whileExpr) noexcept;
  WhileExpr& operator=(const WhileExpr& whileExpr);
  WhileExpr& operator=(WhileExpr&& whileExpr) noexcept;
  ~WhileExpr();
};
// Get expression - an expression and a name
struct GetExpr {
  std::unique_ptr<Expr> expr;
  LiteralExpr name;
  GetExpr() : expr(nullptr), name("") {}
  GetExpr(Expr expr, LiteralExpr name);
  GetExpr(const GetExpr& getExpr);
  GetExpr(GetExpr&& getExpr) noexcept;
  GetExpr& operator=(const GetExpr& getExpr);
  GetExpr& operator=(GetExpr&& getExpr) noexcept;
  ~GetExpr();
};
// Call expression - an expression with parameters
struct CallExpr {
  std::unique_ptr<Expr> expr;
  std::vector<std::unique_ptr<Expr>> params;
  CallExpr() = default;
  CallExpr(const CallExpr& callExpr);
  CallExpr(CallExpr&& callExpr) noexcept;
  CallExpr& operator=(const CallExpr& callExpr);
  CallExpr& operator=(CallExpr&& callExpr) noexcept;
  ~CallExpr();
};
// Function expression - parameters, number of parameters, return type, and an
// action
struct FunctionExpr {
  int arity;
  std::unique_ptr<Environment> parameters;
  std::shared_ptr<Type> returnType;
  std::unique_ptr<Expr> action;
  FunctionExpr() = default;
  FunctionExpr(const FunctionExpr& functionExpr);
  FunctionExpr(FunctionExpr&& functionExpr) noexcept;
  FunctionExpr& operator=(const FunctionExpr& functionExpr);
  FunctionExpr& operator=(FunctionExpr&& functionExpr) noexcept;
  ~FunctionExpr();
};
// Every expression is one of the preceding types
using InnerExpr =
    std::variant<BinaryExpr, PrefixExpr, IntExpr, FloatExpr, BoolExpr, CharExpr,
                 StringExpr, LiteralExpr, FunctionExpr, TypeConvExpr, MatchExpr,
                 IfExpr, BlockExpr, ForExpr, WhileExpr, GetExpr, CallExpr>;
// Main expression information
struct Expr {
  SourceLocation sourceLocation;
  std::shared_ptr<Type> type;
  InnerExpr innerExpr;
  // Accept a new inner expression
  void accept(const InnerExpr& inner_expr) { this->innerExpr = inner_expr; }
  template <typename R>
  // Visit the expression based on its inner type
  R accept(ExprVisitor<R>* visitor) {
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
          } else if (std::is_same_v<T, BoolExpr>) {
            return visitor->visitBoolExpr(this);
          } else if (std::is_same_v<T, CharExpr>) {
            return visitor->visitCharExpr(this);
          } else if (std::is_same_v<T, StringExpr>) {
            return visitor->visitStringExpr(this);
          } else if (std::is_same_v<T, LiteralExpr>) {
            return visitor->visitLiteralExpr(this);
          } else if (std::is_same_v<T, FunctionExpr>) {
            return visitor->visitFunctionExpr(this);
          } else if (std::is_same_v<T, TypeConvExpr>) {
            return visitor->visitTypeConvExpr(this);
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
  // Clone a pointer to this expression
  [[nodiscard]] std::unique_ptr<Expr> clone() const noexcept {
    return std::make_unique<Expr>(sourceLocation, type, innerExpr);
  }
  // Make an expression of various types
  static Expr makeBinary(const Token& op, std::shared_ptr<Type> type);
  static Expr makePrefix(const Token& op, std::shared_ptr<Type> type);
  static Expr makeInt(const Token& op, std::shared_ptr<Type> type, int num);
  static Expr makeFloat(const Token& op, std::shared_ptr<Type> type,
                        double num);
  static Expr makeTypeConv(const SourceLocation& source_location,
                           std::shared_ptr<Type> from,
                           std::shared_ptr<Type> to);
  // Return an expression of various types
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
  [[nodiscard]] BoolExpr* getBoolExpr() {
    return &std::get<BoolExpr>(innerExpr);
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

  [[nodiscard]] FunctionExpr* getFunctionExpr() {
    return &std::get<FunctionExpr>(innerExpr);
  }
  [[nodiscard]] TypeConvExpr* getTypeConvExpr() {
    return &std::get<TypeConvExpr>(innerExpr);
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

  // Get the value of an int or float expr
  [[nodiscard]] int getInt() const { return std::get<IntExpr>(innerExpr).val; }
  [[nodiscard]] double getFloat() const {
    return std::get<FloatExpr>(innerExpr).val;
  }

  // Check if expression is of various types
  [[nodiscard]] bool isBinaryExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, BinaryExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isPrefixExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, PrefixExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isIntExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, IntExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isFloatExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, FloatExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isBoolExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, BoolExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isCharExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, CharExpr>;
        },
        innerExpr);
  }

  [[nodiscard]] bool isStringExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, StringExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isLiteralExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, LiteralExpr>;
        },
        innerExpr);
  }

  [[nodiscard]] bool isFunctionExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, FunctionExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isTypeConvExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, TypeConvExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isMatchExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, MatchExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isIfExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, IfExpr>;
        },
        innerExpr);
  }

  [[nodiscard]] bool isBlockExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, BlockExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isForExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, ForExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isWhileExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, WhileExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isGetExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, GetExpr>;
        },
        innerExpr);
  }
  [[nodiscard]] bool isCallExpr() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, CallExpr>;
        },
        innerExpr);
  }

  // Default constructor
  Expr(const Expr& expr);
  Expr(Expr&& expr) noexcept;
  Expr(const SourceLocation& source_location, std::shared_ptr<Type> type,
       const InnerExpr& inner_expr);
  ~Expr();
};

template <typename T>
// Visitor for an expression
struct ExprVisitor {
  // Visit an expression - return ans from accepting, if type is not void
  T visitExpr(Expr* expr) {
    enterExprVisitor();
    if constexpr (std::is_same_v<T, void>) {
      expr->accept(this);
      exitExprVisitor();
    } else {
      T ans = expr->accept(this);
      exitExprVisitor();
      return ans;
    }
  }
  T _visitExpr(Expr* expr) { return expr->accept(this); }
  // Pure virtual functions
  virtual void enterExprVisitor() = 0;
  virtual void exitExprVisitor() = 0;

  virtual T visitBinaryExpr(Expr* binaryExpr) = 0;
  virtual T visitPrefixExpr(Expr* prefixExpr) = 0;
  virtual T visitIntExpr(Expr* intExpr) = 0;
  virtual T visitFloatExpr(Expr* floatExpr) = 0;
  virtual T visitBoolExpr(Expr* boolExpr) = 0;
  virtual T visitCharExpr(Expr* charExpr) = 0;
  virtual T visitStringExpr(Expr* stringExpr) = 0;
  virtual T visitLiteralExpr(Expr* literalExpr) = 0;
  virtual T visitFunctionExpr(Expr* functionExpr) = 0;
  virtual T visitTypeConvExpr(Expr* TypeConvExpr) = 0;
  virtual T visitMatchExpr(Expr* matchExpr) = 0;
  virtual T visitIfExpr(Expr* ifExpr) = 0;
  virtual T visitBlockExpr(Expr* blockExpr) = 0;
  virtual T visitForExpr(Expr* forExpr) = 0;
  virtual T visitWhileExpr(Expr* whileExpr) = 0;
  virtual T visitGetExpr(Expr* getExpr) = 0;
  virtual T visitCallExpr(Expr* callExpr) = 0;
};
#endif  // INCLUDE_SRC_EXPR_H_
