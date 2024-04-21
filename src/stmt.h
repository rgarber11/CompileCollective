// Copyright (c) 2024 Compile Collective. All Rights Reserved.

#ifndef SENIORPROJECT_STMT_H
#define SENIORPROJECT_STMT_H
#include <memory>
#include <type_traits>
#include <vector>

#include "common.h"
#include "expr.h"
#include "types.h"
template <typename T>
struct StmtVisitor;
struct Stmt;
// Declaration statement - whether const is used, name of the variable, value
struct DeclarationStmt {
  bool consted;
  std::string name;
  std::unique_ptr<Expr> val;
  DeclarationStmt() = default;
  DeclarationStmt(const DeclarationStmt& declaration_stmt);
  DeclarationStmt(DeclarationStmt&& declaration_stmt) noexcept;
  DeclarationStmt& operator=(DeclarationStmt&& other) noexcept;
  DeclarationStmt& operator=(const DeclarationStmt& other);
  ~DeclarationStmt();
};
// Return statement - value
struct ReturnStmt {
  std::unique_ptr<Expr> val;
};
// Yield statement - value
struct YieldStmt {
  std::unique_ptr<Expr> val;
};
// Expression statement - value
struct ExprStmt {
  std::unique_ptr<Expr> val;
};
// Class statement - name, parameters, type
struct ClassStmt {
  std::string name;
  std::vector<Stmt> parameters;
  std::shared_ptr<Type> structType;
  ClassStmt() = default;
  ClassStmt(const ClassStmt& class_stmt);
  ClassStmt(ClassStmt&& class_stmt) noexcept;
  ClassStmt& operator=(const ClassStmt& other);
  ClassStmt& operator=(ClassStmt&& other) noexcept;
  ~ClassStmt();
};
// Impl statement - name, whether decorating, parameters, and type
struct ImplStmt {
  std::string name;
  std::string decorating;
  std::vector<Stmt> parameters;
  std::shared_ptr<Type> implType;
  ImplStmt() = default;
  ImplStmt(const ImplStmt& impl_stmt);
  ImplStmt(ImplStmt&& impl_stmt) noexcept;
  ImplStmt& operator=(const ImplStmt& other);
  ImplStmt& operator=(ImplStmt&& other) noexcept;
  ~ImplStmt();
};
// Type definition - type
struct TypeDef {
  std::shared_ptr<Type> type;
};
// Continue statement
struct ContinueStmt {};
// Every statement is one of the preceding types
using InnerStmt =
    std::variant<ContinueStmt, DeclarationStmt, ReturnStmt, YieldStmt, ExprStmt,
                 ClassStmt, ImplStmt, TypeDef>;
// Statement
struct Stmt {
  // Type and location
  SourceLocation location;
  std::shared_ptr<Type> type;
  InnerStmt stmt;
  // Clone a pointer to this statement
  [[nodiscard]] std::unique_ptr<Stmt> clone() const {
    return std::make_unique<Stmt>(*this);
  }
  // Constructors and destructor
  Stmt() = default;
  Stmt(const Stmt& stmt);
  Stmt(Stmt&& stmt) noexcept;
  ~Stmt();
  // Check if statement is of various types
  bool isDeclarationStmt() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, DeclarationStmt>;
        },
        stmt);
  }
  bool isReturnStmt() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, ReturnStmt>;
        },
        stmt);
  }
  bool isYieldStmt() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, YieldStmt>;
        },
        stmt);
  }
  bool isExprStmt() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, ExprStmt>;
        },
        stmt);
  }
  bool isClassStmt() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, ClassStmt>;
        },
        stmt);
  }
  bool isImplStmt() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, ImplStmt>;
        },
        stmt);
  }

  bool isTypeDef() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, TypeDef>;
        },
        stmt);
  }
  bool isContinueStmt() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, ContinueStmt>;
        },
        stmt);
  }
  // Return a statement of various types
  DeclarationStmt* getDeclarationStmt() {
    return &std::get<DeclarationStmt>(stmt);
  }
  ReturnStmt* getReturnStmt() { return &std::get<ReturnStmt>(stmt); }
  YieldStmt* getYieldStmt() { return &std::get<YieldStmt>(stmt); }
  ExprStmt* getExprStmt() { return &std::get<ExprStmt>(stmt); }
  ClassStmt* getClassStmt() { return &std::get<ClassStmt>(stmt); }
  ImplStmt* getImplStmt() { return &std::get<ImplStmt>(stmt); }
  TypeDef* getTypeDef() { return &std::get<TypeDef>(stmt); }
  ContinueStmt* getContinueStmt() { return &std::get<ContinueStmt>(stmt); }
  // Get string name, based on type, and "" if none
  std::string getName() {
    return std::visit(
        [](auto&& arg) -> std::string {
          using currType = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<currType, DeclarationStmt>) {
            return arg.name;
          } else if constexpr (std::is_same_v<ClassStmt, currType>) {
            return arg.name;
          } else if constexpr (std::is_same_v<TypeDef, currType>) {
            return arg.type->getAliasType()->alias;
          } else if constexpr (std::is_same_v<ImplStmt, currType>) {
            if (arg.decorating.empty()) {
              return arg.name;
            } else {
              return "$" + arg.name + "$" + arg.decorating;
            }
          }
          return "";
        },
        stmt);
  };
  template <typename R>
  // Visit the statement based on its inner type
  R accept(StmtVisitor<R>* visitor) {
    return std::visit(
        [visitor, this](auto&& arg) -> R {
          using T = std::decay_t<decltype(arg)>;
          if (std::is_same_v<T, ContinueStmt>) {
            return visitor->visitContinueStmt(this);
          } else if (std::is_same_v<T, DeclarationStmt>) {
            return visitor->visitDeclarationStmt(this);
          } else if (std::is_same_v<T, ReturnStmt>) {
            return visitor->visitReturnStmt(this);
          } else if (std::is_same_v<T, YieldStmt>) {
            return visitor->visitYieldStmt(this);
          } else if (std::is_same_v<T, ExprStmt>) {
            return visitor->visitExprStmt(this);
          } else if (std::is_same_v<T, ClassStmt>) {
            return visitor->visitClassStmt(this);
          } else if (std::is_same_v<T, ImplStmt>) {
            return visitor->visitImplStmt(this);
          } else if (std::is_same_v<T, TypeDef>) {
            return visitor->visitTypeDef(this);
          }
        },
        stmt);
  }
};
template <typename T>
// Visitor for a statement
struct StmtVisitor {
  // Visit a statement - return ans from accepting, if type is not void
  T visitStmt(Stmt* stmt) {
    enterStmtVisitor();
    if constexpr (std::is_same_v<T, void>) {
      stmt->accept(this);
      exitStmtVisitor();
    } else {
      T ans = stmt->accept(this);
      exitStmtVisitor();
      return ans;
    }
  }
  T _visitStmt(Stmt* stmt) { return stmt->accept(this); }
  // Pure virtual functions
  virtual void enterStmtVisitor() = 0;
  virtual void exitStmtVisitor() = 0;
  virtual T visitContinueStmt(Stmt* continueStmt) = 0;
  virtual T visitDeclarationStmt(Stmt* declarationStmt) = 0;
  virtual T visitReturnStmt(Stmt* returnStmt) = 0;
  virtual T visitYieldStmt(Stmt* yieldStmt) = 0;
  virtual T visitExprStmt(Stmt* exprStmt) = 0;
  virtual T visitClassStmt(Stmt* classStmt) = 0;
  virtual T visitImplStmt(Stmt* implStmt) = 0;
  virtual T visitTypeDef(Stmt* typeDef) = 0;
};
#endif  // SENIORPROJECT_STMT_H
