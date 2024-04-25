// Copyright (c) 2024 Compile Collective. All Rights Reserved.

#ifndef SENIORPROJECT_TYPES_H
#define SENIORPROJECT_TYPES_H
#include <memory>
#include <utility>
#include <variant>
#include <vector>
struct Type;
// Conversion possibilities
enum class Convert { SAME, IMPLICIT, EXPLICIT, FALSE };
// Primary types
enum class BottomType { INT, CHAR, BOOL, FLOAT, VOID, SELF };
// Optional type (may or may not have value)
struct OptionalType {
  std::shared_ptr<Type> optional;
  OptionalType(std::shared_ptr<Type> optional_type);
  OptionalType(const OptionalType& optional_type);
  OptionalType(OptionalType&& optional_type) noexcept;
};
// Tuple type (collection of types)
struct TupleType {
  std::vector<std::shared_ptr<Type>> types;
};
// List type - size and type
struct ListType {
  int size;
  std::shared_ptr<Type> type;
  ListType(int size, std::shared_ptr<Type> type);
  ListType(const ListType& list_type);
  ListType(ListType&& list_type) noexcept;
};
// Sum type - list of types
struct SumType {
  std::vector<std::shared_ptr<Type>> types;
};
// Function type - parameters and return type
struct FunctionType {
  std::shared_ptr<Type> returner;
  std::vector<std::shared_ptr<Type>> parameters;
  FunctionType();
  FunctionType(std::shared_ptr<Type> type,
               std::vector<std::shared_ptr<Type>> params)
      : returner(type), parameters(params){};
  FunctionType(const FunctionType& function_type);
  FunctionType(FunctionType&& function_type) noexcept;
};
// Alias type - alias and type
struct AliasType {
  std::string alias;
  std::shared_ptr<Type> type;
  AliasType(const std::string_view name) : alias(name){};
  AliasType(const std::string_view name, std::shared_ptr<Type> t)
      : alias(name), type(std::move(t)){};
  AliasType(const AliasType& alias_type);
  AliasType(AliasType&& alias_type) noexcept;
};
// Struct type - vector of alias types
struct StructType {
  std::vector<AliasType> types;
};
// Impl - list of alias types it includes
struct Impl {
  std::vector<AliasType> includes;
};
// Every expression is one of the preceding types
using InnerType =
    std::variant<BottomType, OptionalType, TupleType, ListType, StructType,
                 SumType, FunctionType, AliasType, Impl>;
// Main type information
struct Type {
  InnerType type;
  std::vector<std::shared_ptr<Impl>> interfaces;
  Type(const InnerType& type,
       const std::vector<std::shared_ptr<Impl>>& interfaces);
  Type(const Type& type_t);
  // Clone a pointer to this type
  [[nodiscard]] std::shared_ptr<Type> clone() const {
    return std::make_shared<Type>(type, interfaces);
  }
  // Check if type is of various types
  bool isBottomType() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, BottomType>;
        },
        type);
  }
  bool isOptionalType() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, OptionalType>;
        },
        type);
  }
  bool isTupleType() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, TupleType>;
        },
        type);
  }
  bool isListType() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, ListType>;
        },
        type);
  }
  bool isStructType() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, StructType>;
        },
        type);
  }
  bool isSumType() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, SumType>;
        },
        type);
  }
  bool isFunctionType() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, FunctionType>;
        },
        type);
  }
  bool isAliasType() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, AliasType>;
        },
        type);
  }
  bool isImpl() const {
    return std::visit(
        [](auto&& arg) {
          return std::is_same_v<std::decay_t<decltype(arg)>, Impl>;
        },
        type);
  }
  // Return a type of various types
  BottomType getBottomType() {  // NOTE BECAUSE ENUM RETURNS VALUE
    return std::get<BottomType>(type);
  };
  OptionalType* getOptionalType() { return &std::get<OptionalType>(type); };
  TupleType* getTupleType() { return &std::get<TupleType>(type); };
  ListType* getListType() { return &std::get<ListType>(type); };
  StructType* getStructType() { return &std::get<StructType>(type); };
  SumType* getSumType() { return &std::get<SumType>(type); };
  Impl* getImpl() { return &std::get<Impl>(type); };
  FunctionType* getFunctionType() { return &std::get<FunctionType>(type); };
  AliasType* getAliasType() { return &std::get<AliasType>(type); };
  Convert isConvertible(Type* t);
  static std::shared_ptr<Type> mergeTypes(std::shared_ptr<Type> a,
                                          std::shared_ptr<Type> b);
};
#endif  // SENIORPROJECT_TYPES_H
