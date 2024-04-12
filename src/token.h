// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef SENIORPROJECT_TOKEN_H_
#define SENIORPROJECT_TOKEN_H_
#include <string>
#include <string_view>

#include "common.h"
enum class TOKEN_TYPE {
  INT,
  PLUS,
  MINUS,
  STAR,
  SLASH,
  FILE_END,
  ERROR,
  LEFT_PAREN,
  RIGHT_PAREN,
  FLOAT,
  LET,
  COMMA,
  CONST,
  FOR,
  WHILE,
  IF,
  LANGLE,
  RANGLE,
  ASSIGN,
  NEQUALS,
  EQUALS,
  GEQ,
  LEQ,
  IDEN,
  TRUE,
  FALSE,
  SEMI,
  RETURN,
  YIELD,
  AND,
  BITAND,
  NOT,
  XOR,
  OR,
  BITOR,
  LSHIFT,
  RSHIFT,
  STRING,
  CHAR,
  FN,
  CLASS,
  IMPL,
  LSQUARE,
  RSQUARE,
  LBRACKET,
  RBRACKET,
  MATCH,
  CASE,
  SELF,
  ELSE,
  IN,
  DOT,
  RANGE,
  INCRANGE,
  TYPE,
  COLON,
  ARROW,
  OPTIONAL,
  LIST,
  MOD,
  CONTINUE,
  VOID
};
std::string debugTokenTypes(TOKEN_TYPE type);
struct Token {
  TOKEN_TYPE type;
  std::string_view text;
  SourceLocation sourceLocation;
};
#endif  // SENIORPROJECT_TOKEN_H__
