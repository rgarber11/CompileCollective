// Copyright (c) 2024 Compile Collective. All Rights Reserved.

#include "stmt.h"

#include <ctime>
Stmt::Stmt(const Stmt& stmt)
    : location(stmt.location), type(stmt.type), stmt(stmt.stmt) {}
Stmt::Stmt(Stmt&& stmt) noexcept
    : location(stmt.location),
      type(std::move(stmt.type)),
      stmt(std::move(stmt.stmt)) {}
Stmt::~Stmt() = default;
ClassStmt::ClassStmt(const ClassStmt& class_stmt)
    : name(class_stmt.name), structType(class_stmt.structType) {
  for (auto& stmt : class_stmt.parameters) {
    this->parameters.emplace_back(stmt);
  }
}
ClassStmt::ClassStmt(ClassStmt&& class_stmt) noexcept
    : name(std::move(class_stmt.name)), structType(class_stmt.structType) {
  for (auto stmt : class_stmt.parameters) {
    this->parameters.emplace_back(std::move(stmt));
  }
}

ClassStmt::~ClassStmt() = default;
ImplStmt::ImplStmt(const ImplStmt& impl_stmt)
    : name(impl_stmt.name),
      decorating(impl_stmt.decorating),
      implType(impl_stmt.implType) {
  for (auto& stmt : impl_stmt.parameters) {
    this->parameters.emplace_back(stmt);
  }
}
ImplStmt::ImplStmt(ImplStmt&& impl_stmt) noexcept
    : name(std::move(impl_stmt.name)),
      decorating(std::move(impl_stmt.decorating)),
      implType(std::move(impl_stmt.implType)) {
  for (auto stmt : impl_stmt.parameters) {
    this->parameters.emplace_back(std::move(stmt));
  }
}
ImplStmt::~ImplStmt() = default;
DeclarationStmt::DeclarationStmt(const DeclarationStmt& declaration_stmt)
    : consted(declaration_stmt.consted),
      name(declaration_stmt.name),
      val(declaration_stmt.val ? declaration_stmt.val->clone() : nullptr) {}
DeclarationStmt::DeclarationStmt(DeclarationStmt&& declaration_stmt) noexcept
    : consted(declaration_stmt.consted),
      name(std::move(declaration_stmt.name)),
      val(std::move(declaration_stmt.val)) {}
DeclarationStmt& DeclarationStmt::operator=(DeclarationStmt&& other) noexcept {
  consted = other.consted;
  name = other.name;
  val = std::move(other.val);
  return *this;
}
DeclarationStmt& DeclarationStmt::operator=(const DeclarationStmt& other) {
  consted = other.consted;
  name = other.name;
  val = other.val ? other.val->clone() : nullptr;
  return *this;
}
DeclarationStmt::~DeclarationStmt() = default;
ClassStmt& ClassStmt::operator=(const ClassStmt& other) {
  name = other.name;
  for (auto& param : other.parameters) {
    parameters.emplace_back(param);
  }
  structType = other.structType;
  return *this;
}
ClassStmt& ClassStmt::operator=(ClassStmt&& other) noexcept {
  name = std::move(other.name);
  for (auto& param : other.parameters) {
    parameters.emplace_back(std::move(param));
  }
  structType = std::move(other.structType);
  return *this;
}

ImplStmt& ImplStmt::operator=(const ImplStmt& other) {
  name = other.name;
  decorating = other.decorating;
  for (auto& param : other.parameters) {
    parameters.emplace_back(param);
  }
  implType = other.implType;
  return *this;
}
ImplStmt& ImplStmt::operator=(ImplStmt&& other) noexcept {
  name = std::move(other.name);
  decorating = std::move(other.decorating);
  for (auto& param : other.parameters) {
    parameters.emplace_back(std::move(param));
  }
  implType = std::move(other.implType);
  return *this;
}

Stmt& Stmt::operator=(const Stmt& other) {
  location = other.location;
  type = other.type;
  stmt = other.stmt;
  return *this;
}
Stmt& Stmt::operator=(Stmt&& other) noexcept {
  location = other.location;
  type = std::move(other.type);
  stmt = std::move(other.stmt);
  return *this;
}

ReturnStmt::ReturnStmt(const ReturnStmt& return_stmt)
    : val(return_stmt.val ? return_stmt.val->clone() : nullptr) {}
ReturnStmt& ReturnStmt::operator=(const ReturnStmt& other) {
  val = other.val ? other.val->clone() : nullptr;
  return *this;
}

YieldStmt::YieldStmt(const YieldStmt& yield_stmt)
    : val(yield_stmt.val ? yield_stmt.val->clone() : nullptr) {}
YieldStmt& YieldStmt::operator=(const YieldStmt& other) {
  val = other.val ? other.val->clone() : nullptr;
  return *this;
}
ExprStmt::ExprStmt(const ExprStmt& expr_stmt)
    : val(expr_stmt.val ? expr_stmt.val->clone() : nullptr) {}
ExprStmt& ExprStmt::operator=(const ExprStmt& other) {
  val = other.val ? other.val->clone() : nullptr;
  return *this;
}
