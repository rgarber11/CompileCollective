// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#ifndef SENIORPROJECT_ENVIRONMENT_H
#define SENIORPROJECT_ENVIRONMENT_H
#include <memory>
#include <optional>
#include <span>
#include <unordered_map>

#include "stmt.h"
#include "types.h"

// Collection of environment information
struct Environment {
  // Possible redeclaration states
  enum class REDECLARATION_STATES { UNIQUE, ALIAS, REDECLARATION };
  // Collection of pointers to primary types
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
  // Return an environment object
  Environment generateInnerEnvironment() {
    return Environment{this, this->bottomTypes,
                       std::unordered_map<std::string, Stmt>{}};
  }
  // Add a name to members and order
  void addMember(std::string name, Stmt environ) {
    members.emplace(name, environ);
    order.emplace_back(name);
  }
  // Return the appropriate redeclaration state
  REDECLARATION_STATES isRedeclaration(const std::string& name) {
    if (members.find(name) != members.end()) {
      return REDECLARATION_STATES::REDECLARATION;
    } else if (prev &&
               prev->isRedeclaration(name) != REDECLARATION_STATES::UNIQUE) {
      return REDECLARATION_STATES::ALIAS;
    }
    return REDECLARATION_STATES::UNIQUE;
  }
  // Return member with name if it exists, perhaps in the prev environment; nullptr if not 
  Stmt* getMember(const std::string& name) {
    if (members.find(name) != members.end()) {
      return &members[name];
    }
    if (!prev) return nullptr;
    return prev->getMember(name);
  }
  // Return a stmt based on numeric index rather than name
  Stmt* getInOrder(size_t elem) {
    return elem < members.size() ? &members.at(order[elem]) : nullptr;
  }
  // Clone a pointer to this environment
  std::unique_ptr<Environment> clone() {
    return std::make_unique<Environment>(this);
  }
};

#endif  // SENIORPROJECT_ENVIRONMENT_H
