//
// Created by rgarber11 on 4/4/24.
//

#include "stmt.h"
Stmt::Stmt(const Stmt& stmt) : location(stmt.location), type(stmt.type), stmt(stmt.stmt) {}
Stmt::Stmt(Stmt&& stmt) noexcept : location(stmt.location), type(std::move(stmt.type)), stmt(std::move(stmt.stmt)) {}
Stmt::~Stmt() = default;
ClassStmt::ClassStmt(const ClassStmt& class_stmt) : name(class_stmt.name), structType(class_stmt.structType){
  for(auto& stmt : class_stmt.parameters) {
    this->parameters.emplace_back(stmt);
  }
}
ClassStmt::ClassStmt(ClassStmt&& class_stmt) noexcept : name(std::move(class_stmt.name)), structType(class_stmt.structType) {
  for(auto stmt : class_stmt.parameters) {
    this->parameters.emplace_back(std::move(stmt));
  }
}

ClassStmt::~ClassStmt() = default;
ImplStmt::ImplStmt(const ImplStmt& impl_stmt) : name(impl_stmt.name), decorating(impl_stmt.decorating), implType(impl_stmt.implType) {
  for(auto& stmt : impl_stmt.parameters) {
    this->parameters.emplace_back(stmt);
  }
}
ImplStmt::ImplStmt(ImplStmt&& impl_stmt) noexcept : name(std::move(impl_stmt.name)), decorating(std::move(impl_stmt.decorating)), implType(std::move(impl_stmt.implType)) {
  for(auto stmt : impl_stmt.parameters) {
    this->parameters.emplace_back(std::move(stmt));
  }
}
ImplStmt::~ImplStmt() = default;
DeclarationStmt::DeclarationStmt(const DeclarationStmt& declaration_stmt) : consted(declaration_stmt.consted), name(declaration_stmt.name), val(declaration_stmt.val ? declaration_stmt.val->clone() : nullptr) {}
DeclarationStmt::DeclarationStmt(DeclarationStmt&& declaration_stmt) noexcept : consted(declaration_stmt.consted), name(std::move(declaration_stmt.name)), val(std::move(declaration_stmt.val)) {}
DeclarationStmt& DeclarationStmt::operator=(DeclarationStmt&& other) noexcept{
    consted = other.consted;
    name = other.name;
    val = std::move(other.val);
    return *this;
}
DeclarationStmt& DeclarationStmt::operator=(const DeclarationStmt& other){
  consted = other.consted;
  name = other.name;
  val = other.val->clone();
  return *this;
}
DeclarationStmt::~DeclarationStmt() = default;
