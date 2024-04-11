//
// Created by rgarber11 on 4/4/24.
//

#ifndef SENIORPROJECT_TYPES_H
#define SENIORPROJECT_TYPES_H
#include <utility>
#include <vector>
#include <variant>
#include <memory>
struct Type;
enum class Convert{SAME, IMPLICIT, EXPLICIT, FALSE};
enum class BottomType { INT, CHAR, BOOL, FLOAT, VOID };
struct OptionalType {
  std::unique_ptr<Type> optional;
  OptionalType(std::unique_ptr<Type> optional_type);
  OptionalType(const OptionalType& optional_type);
  OptionalType(OptionalType&& optional_type) noexcept;
};
struct TupleType {
  std::vector<Type> types;
};
struct ListType {
  std::unique_ptr<Type> type;
  ListType(std::unique_ptr<Type> type);
  ListType(const ListType& list_type);
  ListType(ListType&& list_type) noexcept;
};
struct SumType {
  std::vector<Type> types;
};
struct FunctionType {
  std::unique_ptr<Type> returner;
  std::vector<Type> parameters;
  FunctionType();
  FunctionType(const FunctionType& function_type);
  FunctionType(FunctionType&& function_type) noexcept;
};
struct AliasType {
  std::string alias;
  std::unique_ptr<Type> type;
  AliasType(const std::string_view  name) : alias(name) {};
  AliasType(const std::string_view name, std::unique_ptr<Type> t) : alias(name), type(std::move(t)) {};
  AliasType(const AliasType& alias_type);
  AliasType(AliasType&& alias_type) noexcept;

};
struct StructType {
  std::vector<AliasType> types;
};
struct Impl {
  std::vector<Type> includes;
};
using InnerType = std::variant<BottomType, OptionalType, TupleType, ListType,
                               StructType, SumType, FunctionType, AliasType, Impl>;
struct Type {
  InnerType type;
  std::vector<Impl> interfaces;
  Type(const InnerType& type, const std::vector<Impl>& interfaces);
  Type(const Type& type_t);
  [[nodiscard]] std::unique_ptr<Type> clone() const {
    return std::make_unique<Type>(type, interfaces);
  }
  bool isBottomType() const {
    return std::visit([](auto&& arg){return std::is_same_v<std::decay<decltype(arg)>, BottomType>;}, type);
  }
  bool isOptionalType() const {
    return std::visit([](auto&& arg){return std::is_same_v<std::decay<decltype(arg)>, OptionalType>;}, type);
  }
  bool isTupleType() const {
    return std::visit([](auto&& arg){return std::is_same_v<std::decay<decltype(arg)>, TupleType>;}, type);
  }
  bool isListType() const {
    return std::visit([](auto&& arg){return std::is_same_v<std::decay<decltype(arg)>, ListType>;}, type);
  }
  bool isStructType() const {
    return std::visit([](auto&& arg){return std::is_same_v<std::decay<decltype(arg)>, StructType>;}, type);
  }
  bool isSumType() const {
    return std::visit([](auto&& arg){return std::is_same_v<std::decay<decltype(arg)>, SumType>;}, type);
  }
  bool isFunctionType() const {
    return std::visit([](auto&& arg){return std::is_same_v<std::decay<decltype(arg)>, FunctionType>;}, type);
  }
  bool isAliasType() const {
    return std::visit([](auto&& arg){return std::is_same_v<std::decay<decltype(arg)>, AliasType>;}, type);
  }
  bool isImpl() const {
    return std::visit([](auto&& arg){return std::is_same_v<std::decay<decltype(arg)>, Impl>;}, type);
  }
  BottomType getBottomType() { //NOTE BECAUSE ENUM RETURNS VALUE
      return std::get<BottomType>(type);
  };
  OptionalType* getOptionalType() {
    return &std::get<OptionalType>(type);
  };
  TupleType* getTupleType() {
    return &std::get<TupleType>(type);
  };
  ListType* getListType() {
    return &std::get<ListType>(type);
  };
  StructType* getStructType() {
    return &std::get<StructType>(type);
  };
  SumType* getSumType() {
    return &std::get<SumType>(type);
  };
  FunctionType* getFunctionType() {
    return &std::get<FunctionType>(type);
  };
  AliasType* getAliasType() {
    return &std::get<AliasType>(type);
  };
  Convert isConvertible(Type* t);
};
#endif  // SENIORPROJECT_TYPES_H