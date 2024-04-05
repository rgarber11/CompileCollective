// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef INCLUDE_SENIORPROJECT_PARSER_H_
#define INCLUDE_SENIORPROJECT_PARSER_H_

#include <memory>
#include <utility>

#include "environment.h"
#include "expr.h"
#include "lexer.h"
#include "stmt.h"
#include "token.h"
#include "types.h"
class Parser {
 private:
  bool munch(TOKEN_TYPE type);
  Environment program;

 public:
  explicit Parser(Lexer lexer) : lexer(std::move(lexer)) {}
  Parser(Parser&& parser) : lexer(parser.lexer), curr(parser.curr) {}
  Parser(const Parser& parser) : lexer(parser.lexer), curr(parser.curr) {}
  ~Parser() = default;
  Environment parse();

 private:
  Lexer lexer;
  Token curr;
  AliasType typeDef();
  Type type();
  Type functionType();
  Type optionalType();
  Type tupleType();
  Type listType();
  Type bottomType();
  Stmt stmt();
  Stmt returnStmt();
  Stmt yieldStmt();
  Stmt functionStmt();
  Stmt exprStmt();
  Stmt implStmt();
  Stmt declarationStmt();
  Stmt classStmt();
  std::unique_ptr<Expr> expr();
  std::unique_ptr<Expr> orExpr();
  std::unique_ptr<Expr> andExpr();
  std::unique_ptr<Expr> bitAndExpr();
  std::unique_ptr<Expr> xorExpr();
  std::unique_ptr<Expr> bitOrExpr();
  std::unique_ptr<Expr> add();
  std::unique_ptr<Expr> mult();
  std::unique_ptr<Expr> primary();
  std::unique_ptr<Expr> access();
  std::unique_ptr<Expr> ifExpr();
  std::unique_ptr<Expr> forExpr();
  std::unique_ptr<Expr> matchExpr();
  std::unique_ptr<Expr> whileExpr();
  std::unique_ptr<Expr> block();
  std::unique_ptr<Expr> prefix();
  Type productType();
};

#endif  // INCLUDE_SENIORPROJECT_PARSER_H_
