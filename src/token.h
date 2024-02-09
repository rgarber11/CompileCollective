// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef SENIORPROJECT_TOKEN_H_
#define SENIORPROJECT_TOKEN_H_
#include <string>
#include <string_view>

#include "common.h"
enum class TOKEN_TYPE {
  NUM,
  PLUS,
  MINUS,
  STAR,
  SLASH,
  FILE_END,
  ERROR,
  LEFT_PAREN,
  RIGHT_PAREN,

};
std::string debugTokenTypes(TOKEN_TYPE type);
struct Token {
  TOKEN_TYPE type;
  std::string_view text;
  SourceLocation sourceLocation;
};
#endif  // SENIORPROJECT_TOKEN_H__
