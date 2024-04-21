#include "types.h"

#include <bit>
#include <memory>
#include <utility>
// Constructors
OptionalType::OptionalType(std::shared_ptr<Type> type)
    : optional(std::move(type)) {}
OptionalType::OptionalType(const OptionalType& optional_type)
    : optional(optional_type.optional ? optional_type.optional->clone()
                                      : nullptr) {}
OptionalType::OptionalType(OptionalType&& optional_type) noexcept
    : optional(optional_type.optional ? optional_type.optional->clone()
                                      : nullptr) {}
ListType::ListType(const ListType& list_type)
    : type(list_type.type ? list_type.type->clone() : nullptr) {}
ListType::ListType(ListType&& list_type) noexcept
    : type(list_type.type ? list_type.type->clone() : nullptr) {}
ListType::ListType(int size, std::shared_ptr<Type> type)
    : size(size), type(type) {}
FunctionType::FunctionType(const FunctionType& function_type)
    : returner(function_type.returner ? function_type.returner->clone()
                                      : nullptr),
      parameters(function_type.parameters) {}
FunctionType::FunctionType(FunctionType&& function_type) noexcept
    : returner(function_type.returner ? function_type.returner->clone()
                                      : nullptr),
      parameters(function_type.parameters) {}
AliasType::AliasType(const AliasType& alias_type)
    : alias(alias_type.alias),
      type(alias_type.type ? alias_type.type->clone() : nullptr) {}
AliasType::AliasType(AliasType&& alias_type) noexcept
    : alias(alias_type.alias),
      type(alias_type.type ? alias_type.type->clone() : nullptr) {}
Type::Type(const Type& type_t)
    : type(type_t.type), interfaces(type_t.interfaces) {}
Type::Type(const InnerType& type,
           const std::vector<std::shared_ptr<Impl>>& interfaces)
    : type(type), interfaces(interfaces) {}
// Return if this type is convertible with another one
Convert Type::isConvertible(Type* t) {
  // Same types
  if (this == t) return Convert::SAME;
  if (t->isBottomType() && t->getBottomType() == BottomType::VOID) {
    // Void
    if (this->isBottomType() && this->getBottomType() == BottomType::VOID)
      return Convert::SAME;
    if (this->isOptionalType()) return Convert::IMPLICIT;
    return Convert::FALSE;
  }
  if (this->isOptionalType()) {
    // Optional
    if (t->isOptionalType()) {
      return this->getOptionalType()->optional->isConvertible(
          t->getOptionalType()->optional.get());
    } else {
      auto ans = this->getOptionalType()->optional->isConvertible(t);
      return ans == Convert::SAME ? Convert::IMPLICIT : ans;
    }
  } else if (this->isBottomType()) {
    // Bottom
    if (this->getBottomType() == BottomType::VOID) return Convert::IMPLICIT;
    if (t->isAliasType()) {
      // Alias
      auto ans = this->isConvertible(t->getAliasType()->type.get());
      return ans == Convert::SAME ? Convert::IMPLICIT : ans;
    }
    if (!t->isBottomType()) return Convert::FALSE;
    switch (this->getBottomType()) {
      // Bottom types are convertible with some others
      case BottomType::INT:
        switch (t->getBottomType()) {
          case BottomType::INT:
            return Convert::SAME;
          case BottomType::CHAR:
            return Convert::IMPLICIT;
          case BottomType::FLOAT:
          default:
            return Convert::FALSE;
        }
      case BottomType::CHAR:
        switch (t->getBottomType()) {
          case BottomType::INT:
            return Convert::EXPLICIT;
          case BottomType::CHAR:
            return Convert::SAME;
          case BottomType::FLOAT:
          default:
            return Convert::FALSE;
        }
      case BottomType::FLOAT:
        switch (t->getBottomType()) {
          case BottomType::INT:
            return Convert::IMPLICIT;
          case BottomType::CHAR:
            return Convert::EXPLICIT;
          case BottomType::FLOAT:
            return Convert::SAME;
          default:
            return Convert::FALSE;
        }
      case BottomType::BOOL:
        if (t->getBottomType() == BottomType::BOOL) return Convert::SAME;
        return Convert::FALSE;
      case BottomType::SELF:
        if (t->isStructType()) return Convert::IMPLICIT;
        return Convert::FALSE;
    }
  } else if (this->isTupleType()) {
    // Tuple
    if (!t->isTupleType() ||
        this->getTupleType()->types.size() != t->getTupleType()->types.size())
      return Convert::FALSE;
    Convert ans = Convert::SAME;
    for (int i = 0; i < this->getTupleType()->types.size(); ++i) {
      auto temp = this->getTupleType()->types[i]->isConvertible(
          t->getTupleType()->types[i].get());
      if (temp == Convert::FALSE) return Convert::FALSE;
      if (temp == Convert::IMPLICIT) {
        ans = ans == Convert::EXPLICIT ? ans : temp;
      } else if (temp == Convert::EXPLICIT) {
        ans = Convert::EXPLICIT;
      }
    }
    return ans;
  } else if (this->isListType()) {
    // List
    if (!t->getListType()) {
      auto ans = this->getListType()->type->isConvertible(t);
      return ans == Convert::FALSE ? Convert::FALSE : Convert::EXPLICIT;
    }
    if (this->getListType()->size == -1) {
      return this->getListType()->type->isConvertible(
          t->getListType()->type.get());
    } else if (t->getListType()->size > this->getListType()->size) {
      return Convert::FALSE;
    } else {
      return this->getListType()->type->isConvertible(
                 t->getListType()->type.get()) == Convert::SAME
                 ? Convert::IMPLICIT
                 : this->getListType()->type->isConvertible(
                       t->getListType()->type.get());
    }
  } else if (this->isStructType()) {
    // Struct
    if (!t->isStructType() ||
        this->getStructType()->types.size() != t->getStructType()->types.size())
      return Convert::FALSE;
    bool expliciter = false;
    for (int i = 0; i < this->getStructType()->types.size(); ++i) {
      if (this->getStructType()->types[i].alias !=
          t->getStructType()->types[i].alias) {
        auto temp = this->getStructType()->types[i].type->isConvertible(
            t->getStructType()->types[i].type.get());
        if (temp == Convert::FALSE || temp == Convert::EXPLICIT) {
          return Convert::FALSE;
        }
        expliciter = true;
      }
    }
    return expliciter ? Convert::EXPLICIT : Convert::SAME;
  } else if (this->isAliasType()) {
    // Alias
    if (t->isAliasType()) {
      if (this->getAliasType()->alias == t->getAliasType()->alias)
        return Convert::SAME;
      auto ans = this->getAliasType()->type->isConvertible(
          t->getAliasType()->type.get());
      return ans == Convert::FALSE ? Convert::FALSE : Convert::EXPLICIT;
    }
    if (!t->isAliasType()) {
      auto ans = this->getAliasType()->type->isConvertible(t);
      return ans == Convert::SAME ? Convert::IMPLICIT : ans;
    }
  } else if (this->isImpl()) {
    // Impl
    return Convert::FALSE;
  } else if (this->isFunctionType()) {
    // Function
    if (!t->isFunctionType()) return Convert::FALSE;
    if (this->getFunctionType()->parameters.size() !=
        t->getFunctionType()->parameters.size())
      return Convert::FALSE;
    auto ans = t->getFunctionType()->returner->isConvertible(
        this->getFunctionType()->returner.get());
    if (ans == Convert::FALSE) return Convert::FALSE;
    for (int i = 0; i < this->getFunctionType()->parameters.size(); ++i) {
      auto temp = t->getFunctionType()->parameters[i]->isConvertible(
          this->getFunctionType()->parameters[i].get());
      if (temp == Convert::FALSE || temp == Convert::EXPLICIT)
        return Convert::FALSE;
      if (temp == Convert::IMPLICIT) ans = Convert::IMPLICIT;
    }
    return ans;
  } else if (this->isSumType()) {
    // Sum
    if (!t->isSumType()) {
      Convert ans = Convert::FALSE;
      for (std::shared_ptr<Type> prod : this->getSumType()->types) {
        auto temp = prod->isConvertible(t);
        if (temp == Convert::SAME || temp == Convert::IMPLICIT)
          return Convert::IMPLICIT;
        if (temp == Convert::EXPLICIT) ans = Convert::EXPLICIT;
      }
      return ans;
    } else {
      bool same = false;
      if (this->getSumType()->types.size() == t->getSumType()->types.size())
        same = true;
      auto ans = Convert::SAME;
      for (std::shared_ptr<Type> prod : t->getSumType()->types) {
        auto temp = this->isConvertible(prod.get());
        if (temp == Convert::FALSE || temp == Convert::EXPLICIT)
          return Convert::FALSE;
        if (temp == Convert::IMPLICIT) ans = Convert::IMPLICIT;
      }
      if (same && ans == Convert::SAME) return Convert::SAME;
      return Convert::IMPLICIT;
    }
  }
  // False by default
  return Convert::FALSE;
}
FunctionType::FunctionType() : returner(nullptr) {}
// Merge two types
std::shared_ptr<Type> Type::mergeTypes(std::shared_ptr<Type> a,
                                       std::shared_ptr<Type> b) {
  // Return either if same types, the one convertible to if implicit conversion possible (a first)
  if (a == b) return a;
  if (a->isConvertible(b.get()) == Convert::SAME ||
      a->isConvertible(b.get()) == Convert::IMPLICIT)
    return a;
  if (b->isConvertible(a.get()) == Convert::SAME ||
      b->isConvertible(a.get()) == Convert::IMPLICIT)
    return b;
  if (a->isSumType() && b->isSumType()) {
    // Both sum type
    size_t aSize = a->getSumType()->types.size();
    size_t bSize = b->getSumType()->types.size();
    std::vector<bool> required(aSize + bSize, true);
    for (int i = 0; i < aSize; ++i) {
      if (!required[i]) continue;
      for (int j = 0; j < bSize; ++j) {
        if (!required[aSize + j]) continue;
        if (a->getSumType()->types[i]->isConvertible(
                b->getSumType()->types[j].get()) == Convert::SAME ||
            a->getSumType()->types[i]->isConvertible(
                b->getSumType()->types[j].get()) == Convert::IMPLICIT) {
          required[aSize + j] = false;
        }
      }
    }
    for (int j = 0; j < bSize; ++j) {
      if (!required[aSize + j]) continue;
      for (int i = 0; i < aSize; ++i) {
        if (!required[i]) continue;
        if (b->getSumType()->types[j]->isConvertible(
                a->getSumType()->types[i].get()) == Convert::SAME ||
            b->getSumType()->types[j]->isConvertible(
                a->getSumType()->types[i].get()) == Convert::IMPLICIT) {
          required[i] = false;
        }
      }
    }
    auto returnType = std::make_shared<Type>(Type{SumType{}, {}});
    for (int i = 0; i < aSize; ++i) {
      if (!required[i]) continue;
      returnType->getSumType()->types.emplace_back(a->getSumType()->types[i]);
    }
    for (int j = 0; j < bSize; ++j) {
      if (!required[aSize + j]) continue;
      returnType->getSumType()->types.emplace_back(b->getSumType()->types[j]);
    }
    return returnType;
  } else if (a->isSumType()) {
    // a only sum type
    for (auto& typ : a->getSumType()->types) {
      if (typ->isConvertible(b.get()) == Convert::SAME ||
          typ->isConvertible(b.get()) == Convert::IMPLICIT) {
      }
      return a;
    }
    return a->clone()->getSumType()->types.emplace_back(b);
  } else if (b->isSumType()) {
    // b only sum type
    for (auto& typ : b->getSumType()->types) {
      if (typ->isConvertible(a.get()) == Convert::SAME ||
          typ->isConvertible(a.get()) == Convert::IMPLICIT) {
      }
      return b;
    }
    return b->clone()->getSumType()->types.emplace_back(a);
  } else {
    // Optional if other is void, sum otherwise
    if (a->isBottomType() && a->getBottomType() == BottomType::VOID) {
      return std::make_shared<Type>(Type{OptionalType(b), {}});
    }
    if (b->isBottomType() && b->getBottomType() == BottomType::VOID) {
      return std::make_shared<Type>(Type{OptionalType(a), {}});
    }
    return std::make_shared<Type>(Type{SumType({a, b}), {}});
  }
}
