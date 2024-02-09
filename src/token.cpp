// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#include "token.h"

std::string debugTokenTypes(TOKEN_TYPE type) {
  switch (type) {
    case TOKEN_TYPE::NUM:
      return "NUM";

    case TOKEN_TYPE::PLUS:
      return "+";

    case TOKEN_TYPE::MINUS:
      return "-";

    case TOKEN_TYPE::STAR:
      return "*";

    case TOKEN_TYPE::SLASH:
      return "/";

    case TOKEN_TYPE::FILE_END:
      return "FILE_END";

    case TOKEN_TYPE::ERROR:
      return "ERROR";

    case TOKEN_TYPE::LEFT_PAREN:
      return "(";

    case TOKEN_TYPE::RIGHT_PAREN:
      return ")";
  }
  return "";
}
