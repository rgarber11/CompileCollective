// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef SENIORPROJECT_ENVIRONMENT_H
#define SENIORPROJECT_ENVIRONMENT_H
#include <memory>
#include <unordered_map>

#include "stmt.h"
#include "types.h"

struct Environment {
  struct BottomTypes {
    std::shared_ptr<Type> voidType;
    std::shared_ptr<Type> intType;
    std::shared_ptr<Type> boolType;
    std::shared_ptr<Type> charType;
    std::shared_ptr<Type> floatType;
  };
  Environment* prev;
  BottomTypes bottomTypes;
  std::unordered_map<std::string, Stmt> members;
  Environment generateInnerEnvironment() {
    return Environment{this, this->bottomTypes,
                       std::unordered_map<std::string, Stmt>{}};
  }
};

#endif  // SENIORPROJECT_ENVIRONMENT_H
