//
// Created by rgarber11 on 4/4/24.
//

#ifndef SENIORPROJECT_STMT_H
#define SENIORPROJECT_STMT_H
#include <memory>
#include <vector>

#include "expr.h"
#include "types.h"
struct Stmt;
struct DeclarationStmt {
  bool consted;
  std::string name;
  std::unique_ptr<Expr> val;
};
struct ReturnStmt {
  std::unique_ptr<Expr> val;
};
struct YieldStmt {
  std::unique_ptr<Expr> val;
};
struct ExprStmt {
  std::unique_ptr<Expr> val;
};
struct ClassStmt {
  std::string name;
  std::vector<DeclarationStmt> parameters;
};
struct ImplStmt {
  std::string name;
  std::string decorating;
  std::vector<DeclarationStmt> parameters;
};
struct TypeDef {
  std::shared_ptr<AliasType> type;
};
struct ContinueStmt {};
using InnerStmt =
    std::variant<ContinueStmt, DeclarationStmt, ReturnStmt, YieldStmt, ExprStmt,
                 ClassStmt, ImplStmt, TypeDef>;
struct Stmt {
  std::shared_ptr<Type> type;
  InnerStmt stmt;
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
  std::string getName() {
    return std::visit(
        [](auto&& arg) -> std::string {
          using currType = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<currType, DeclarationStmt>) {
            return arg.name;
          } else if constexpr (std::is_same_v<ClassStmt, currType>) {
            return arg.name;
          } else if constexpr (std::is_same_v<TypeDef, currType>) {
            return arg.type->alias;
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
};
#endif  // SENIORPROJECT_STMT_H
