// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef INCLUDE_SRC_COMMON_H_
#define INCLUDE_SRC_COMMON_H_
#include <string_view>
#include <iostream>
// Allow line and character to be accessible anywhere in the code
struct SourceLocation {
  int line;
  int character;
};

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
void exitError(const SourceLocation& loc, const std::string_view& str);
#endif  // INCLUDE_SRC_COMMON_H_
