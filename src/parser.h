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
  enum class state { NORMAL, IMPL, CLASS };
  state isImplClass = Parser::state::NORMAL;
  bool requireNext(TOKEN_TYPE type);
  bool munch(TOKEN_TYPE type);

 public:
  Environment program;
  explicit Parser(Lexer lexer) : lexer(std::move(lexer)) { setup(); }
  Parser(Parser&& parser)
      : lexer(parser.lexer), curr(parser.curr), program(parser.program) {}
  Parser(const Parser& parser)
      : lexer(parser.lexer), curr(parser.curr), program(parser.program) {}
  ~Parser() = default;
  Environment parse();

 private:
  void setup();
  Lexer lexer;
  Token curr;
  Stmt typeDef();
  std::shared_ptr<Type> type();
  std::shared_ptr<Type> functionType();
  std::shared_ptr<Type> optionalType();
  std::shared_ptr<Type> tupleType();
  std::shared_ptr<Type> listType();
  std::shared_ptr<Type> bottomType();
  Stmt stmt();
  Stmt returnStmt();
  Stmt yieldStmt();
  std::unique_ptr<Expr> functionExpr();
  Stmt exprStmt();
  Stmt implStmt();
  Stmt declarationStmt();
  Stmt classStmt();
  std::unique_ptr<Expr> rangeExpr();
  std::unique_ptr<Expr> expr();
  std::unique_ptr<Expr> assign();
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
  ForConditionExpr forConditionExpr();
  std::shared_ptr<Type> productType();
  Stmt globals();
  CaseExpr caseExpr();
};

#endif  // INCLUDE_SENIORPROJECT_PARSER_H_
