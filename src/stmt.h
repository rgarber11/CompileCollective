//
// Created by rgarber11 on 4/4/24.
//

#ifndef SENIORPROJECT_STMT_H
#define SENIORPROJECT_STMT_H
#include <memory>

#include "expr.h"
#include "types.h"
struct Stmt;
struct DeclarationStmt {
  bool consted;
  std::string name;
  std::unique_ptr<Stmt> val;
};
struct ReturnStmt {
  std::unique_ptr<Stmt> val;
};
struct YieldStmt {
  std::unique_ptr<Stmt> val;
};
struct ExprStmt {
  std::unique_ptr<Expr> val;
};
struct FunctionStmt {
  int arity;
  std::vector<std::pair<Type*, std::string>> parameters;
  std::unique_ptr<Stmt> action;
};
struct ClassStmt {
  std::vector<Stmt> parameters;
};
struct ImplStmt {
  std::vector<Stmt> parameters;
};
using InnerStmt = std::variant<DeclarationStmt,  ReturnStmt,
                               YieldStmt, ExprStmt, FunctionStmt, ClassStmt, ImplStmt>;
struct Stmt {
  std::shared_ptr<Type> type;
  InnerStmt stmt;
  bool isDeclarationStmt() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, DeclarationStmt>;
        },
        stmt);
  }
  bool isReturnStmt() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, ReturnStmt>;
        },
        stmt);
  }
  bool isYieldStmt() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, YieldStmt>;
        },
        stmt);
  }
  bool isExprStmt() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, ExprStmt>;
        },
        stmt);
  }
  bool isFunctionStmt() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, FunctionStmt>;
        },
        stmt);
  }
  bool isClassStmt() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, ClassStmt>;
        },
        stmt);
  }
  bool isImplStmt() {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay<decltype(arg)>, ImplStmt>;
        },
        stmt);
  }

  DeclarationStmt* getDeclarationStmt() {
    return &std::get<DeclarationStmt>(stmt);
  }
  ReturnStmt* getReturnStmt() { return &std::get<ReturnStmt>(stmt); }
  YieldStmt* getYieldStmt() { return &std::get<YieldStmt>(stmt); }
  ExprStmt* getExprStmt() { return &std::get<ExprStmt>(stmt); }
  FunctionStmt* getFunctionStmt() { return &std::get<FunctionStmt>(stmt); }
  ClassStmt* getClassStmt() { return &std::get<ClassStmt>(stmt); }
  ImplStmt* getImplStmt() { return &std::get<ImplStmt>(stmt); }
};
#endif  // SENIORPROJECT_STMT_H
