// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef INCLUDE_SENIORPROJECT_PARSER_H_
#define INCLUDE_SENIORPROJECT_PARSER_H_

#include <memory>
#include <optional>
#include <utility>

#include "environment.h"
#include "expr.h"
#include "lexer.h"
#include "stmt.h"
#include "token.h"
#include "types.h"
// Parser class - analyze tokens
class Parser {
 private:
  // State of parser
  enum class state { NORMAL, IMPL, CLASS };
  state isImplClass = Parser::state::NORMAL;
  bool requireNext(TOKEN_TYPE type);
  bool munch(TOKEN_TYPE type);
  bool eatCurr(TOKEN_TYPE type);
  bool inLoop = false;
  bool inBlock = false;

 public:
  // Parser types
  enum class parser { PROGRAM, EXPR, TYPE };
  // Program environment
  std::unique_ptr<Environment> program;
  // Constructors - use a lexer
  explicit Parser(Lexer lexer) : lexer(std::move(lexer)) { setup(); }
  Parser(Lexer lexer, Environment* program)
      : program(program), lexer(std::move(lexer)){};
  Parser(Parser&& parser)
      : program(std::move(parser.program)), lexer(parser.lexer), curr(parser.curr) {}
  Parser(const Parser& parser)
      : program(parser.program->clone()), lexer(parser.lexer), curr(parser.curr) {}
  // Default destructor
  ~Parser() = default;
  std::unique_ptr<Environment> parse(parser is = Parser::parser::PROGRAM);

 private:
  void setup();
  // Lexer, current token
  Lexer lexer;
  Token curr;
  std::optional<Stmt> typeDef();
  std::shared_ptr<Type> type();
  std::shared_ptr<Type> functionType();
  std::shared_ptr<Type> optionalType();
  std::shared_ptr<Type> tupleType();
  std::shared_ptr<Type> listType();
  std::shared_ptr<Type> bottomType();
  std::optional<Stmt> stmt();
  std::optional<Stmt> returnStmt();
  std::optional<Stmt> yieldStmt();
  std::unique_ptr<Expr> functionExpr();
  std::optional<Stmt> exprStmt();
  std::optional<Stmt> implStmt();
  std::optional<Stmt> declarationStmt();
  std::optional<Stmt> classStmt();
  std::unique_ptr<Expr> rangeExpr();
  std::unique_ptr<Expr> expr();
  std::unique_ptr<Expr> assign();
  std::unique_ptr<Expr> orExpr();
  std::unique_ptr<Expr> andExpr();
  std::unique_ptr<Expr> bitAndExpr();
  std::unique_ptr<Expr> equateExpr();

  std::unique_ptr<Expr> xorExpr();
  std::unique_ptr<Expr> bitOrExpr();
  std::unique_ptr<Expr> relation();
  std::unique_ptr<Expr> shift();
  std::unique_ptr<Expr> add();
  std::unique_ptr<Expr> mult();
  std::unique_ptr<Expr> primary();
  std::unique_ptr<Expr> access();
  std::unique_ptr<Expr> ifExpr();
  std::unique_ptr<Expr> forExpr();
  std::unique_ptr<Expr> matchExpr();
  std::unique_ptr<Expr> whileExpr();
  std::unique_ptr<Expr> block();
  std::unique_ptr<Expr> negate();
  std::unique_ptr<Expr> notExpr();
  std::optional<Stmt> forConditionExpr();
  std::shared_ptr<Type> productType();
  std::optional<Stmt> globals();
  CaseExpr caseExpr();
};

#endif  // INCLUDE_SENIORPROJECT_PARSER_H_
