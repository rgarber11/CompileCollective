// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef INCLUDE_SENIORPROJECT_PARSER_H_
#define INCLUDE_SENIORPROJECT_PARSER_H_

#include <memory>
#include <utility>

#include "expr.h"
#include "lexer.h"
#include "token.h"
class Parser {
 public:
  explicit Parser(Lexer lexer) : lexer(std::move(lexer)) {}
  Parser(Parser&& parser) : lexer(parser.lexer), curr(parser.curr) {}
  Parser(const Parser& parser) : lexer(parser.lexer), curr(parser.curr) {}
  ~Parser() = default;
  std::unique_ptr<Expr> parse();

 private:
  Lexer lexer;
  Token curr;
  std::unique_ptr<Expr> expr();
  std::unique_ptr<Expr> add();
  std::unique_ptr<Expr> mult();
  std::unique_ptr<Expr> primary();
  std::unique_ptr<Expr> prefix();
};

#endif  // INCLUDE_SENIORPROJECT_PARSER_H_
