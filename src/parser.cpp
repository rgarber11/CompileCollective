// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#include "parser.h"

#include <charconv>
#include <iostream>
#include <memory>
#include <utility>

#include "expr.h"
#include "token.h"
std::unique_ptr<Expr> Parser::parse() {
  curr = lexer.next();
  auto ans = expr();
  return ans;
}
std::unique_ptr<Expr> Parser::expr() { return add(); }
std::unique_ptr<Expr> Parser::add() {
  std::unique_ptr<Expr> expr = mult();
  while (curr.type == TOKEN_TYPE::PLUS || curr.type == TOKEN_TYPE::MINUS) {
    std::unique_ptr<Expr> binary =
        std::make_unique<Expr>(Expr::makeBinary(curr, TOKEN_TYPE::INT));
    curr = lexer.next();
    binary->getBinary()->left = std::move(expr);
    binary->type = binary->getBinary()->left->type;
    binary->getBinary()->right = mult();
    expr = std::move(binary);
  }
  return expr;
}
std::unique_ptr<Expr> Parser::mult() {
  std::unique_ptr<Expr> expr = prefix();
  while (curr.type == TOKEN_TYPE::STAR || curr.type == TOKEN_TYPE::SLASH) {
    std::unique_ptr<Expr> binary =
        std::make_unique<Expr>(Expr::makeBinary(curr, TOKEN_TYPE::INT));
    curr = lexer.next();
    binary->getBinary()->left = std::move(expr);
    binary->type = binary->getBinary()->left->type;
    binary->getBinary()->right = prefix();
    expr = std::move(binary);
  }
  return expr;
}
std::unique_ptr<Expr> Parser::prefix() {
  if (curr.type == TOKEN_TYPE::MINUS) {
    std::unique_ptr<Expr> expr = std::make_unique<Expr>(Expr::makePrefix(curr, TOKEN_TYPE::INT));
    curr = lexer.next();
    expr->getPrefix()->expr = primary();
    expr->type = expr->getPrefix()->expr->type;
    return expr;
  } else {
    return primary();
  }
}
std::unique_ptr<Expr> Parser::primary() {
  switch (curr.type) {
    case TOKEN_TYPE::INT: {
      int val{};
      auto result = std::from_chars(curr.text.data(),
                                    curr.text.data() + curr.text.size(), val);
      if (result.ec == std::errc::invalid_argument) {
        std::cerr << "Could not convert to int at: " << curr.sourceLocation.line
                  << ":" << curr.sourceLocation.character << ".\n";
        return {};
      }
      std::unique_ptr<Expr> expr =
          std::make_unique<Expr>(Expr::makeInt(curr, val));
      curr = lexer.next();
      return expr;
    }
    case TOKEN_TYPE::FLOAT: {
      double val{};
      auto result = std::from_chars(curr.text.data(),
          curr.text.data() + curr.text.size(), val);
      if (result.ec == std::errc::invalid_argument) {
        std::cerr << "Could not convert to int at: " << curr.sourceLocation.line
                  << ":" << curr.sourceLocation.character << ".\n";
        return {};
      }
      std::unique_ptr<Expr> expr =
          std::make_unique<Expr>(Expr::makeFloat(curr, val));
      curr = lexer.next();
      return expr;
    }
    case TOKEN_TYPE::LEFT_PAREN: {
      curr = lexer.next();
      auto result = expr();
      if (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
        std::cerr << "Parentheses not closed at: " << curr.sourceLocation.line
                  << ":" << curr.sourceLocation.character << '\n';
        return {};
      }
      curr = lexer.next();
      return result;
    }
    default:
      std::cerr << "Invalid token at: " << curr.sourceLocation.line << ":"
                << curr.sourceLocation.character << "!\n";
      return {};
  }
}
