// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef SENIORPROJECT_ENVIRONMENT_H
#define SENIORPROJECT_ENVIRONMENT_H
#include <memory>
#include <unordered_map>

#include "stmt.h"
#include "types.h"

struct Environment {
  enum class REDECLARATION_STATES { UNIQUE, ALIAS, REDECLARATION };
  struct BottomTypes {
    std::shared_ptr<Type> voidType;
    std::shared_ptr<Type> intType;
    std::shared_ptr<Type> boolType;
    std::shared_ptr<Type> charType;
    std::shared_ptr<Type> floatType;
    std::shared_ptr<Type> selfType;
  };
  Environment* prev;
  BottomTypes bottomTypes;
  std::unordered_map<std::string, Stmt> members;
  std::vector<std::string> order;
  Environment generateInnerEnvironment() {
    return Environment{this, this->bottomTypes,
                       std::unordered_map<std::string, Stmt>{}};
  }
  void addMember(std::string name, Stmt environ) {
    members.emplace(name, environ);
    order.emplace_back(name);
  }
  REDECLARATION_STATES isRedeclaration(const std::string& name) {
    if (members.find(name) != members.end()) {
      return REDECLARATION_STATES::REDECLARATION;
    } else if (prev &&
               prev->isRedeclaration(name) != REDECLARATION_STATES::UNIQUE) {
      return REDECLARATION_STATES::ALIAS;
    }
    return REDECLARATION_STATES::UNIQUE;
  }
  Stmt* getMember(const std::string& name) {
    if (members.find(name) != members.end()) {
      return &members[name];
    }
    if (!prev) return nullptr;
    return prev->getMember(name);
  }
};

#endif  // SENIORPROJECT_ENVIRONMENT_H
