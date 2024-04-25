//
// Created by rgarber11 on 4/24/24.
//
#include "common.h"
void exitError(const SourceLocation& loc, const std::string_view& str) {
  std::cout << '[' << loc.line << ':' << loc.character << ']' << ' ' << str << '\n';
  exit(-1);
}
