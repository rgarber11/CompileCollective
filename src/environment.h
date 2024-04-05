//
// Created by rgarber11 on 4/5/24.
//

#ifndef SENIORPROJECT_ENVIRONMENT_H
#define SENIORPROJECT_ENVIRONMENT_H
#include "types.h"
#include "stmt.h"
#include <utility>
#include <unordered_map>

struct Environment {
  Environment* prev;
  std::vector<AliasType> types;
  std::vector<Stmt> globals;
  std::unordered_map<std::string, std::pair<bool, int>> mapper;
  Environment() = default;
  Environment(const Environment& program) = default;
  Environment(Environment&& program) = default;
  ~Environment() = default;
};



#endif //SENIORPROJECT_ENVIRONMENT_H
