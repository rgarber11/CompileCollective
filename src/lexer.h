// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef INCLUDE_SRC_LEXER_H_
#define INCLUDE_SRC_LEXER_H_

#include <cstddef>
#include <string_view>

#include "common.h"
#include "token.h"
class Lexer {
 public:
  explicit Lexer(const std::string_view input)
      : input(input), curr{1, 1}, pos{0} {}
  Lexer(Lexer&& lexer) noexcept
      : input(lexer.input), curr(lexer.curr), pos(lexer.pos) {}
  Lexer(const Lexer& lexer)
      : input(lexer.input), curr(lexer.curr), pos(lexer.pos) {}
  ~Lexer() = default;

  Token next();

 private:
  const std::string_view input;  // Non-owning reference to data
  SourceLocation curr;
  size_t pos;
  void skipWhitespace();
};

#endif  // INCLUDE_SRC_LEXER_H_
