// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#include "parser.h"

#include <pthread.h>

#include <charconv>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "environment.h"
#include "expr.h"
#include "token.h"
#include "types.h"
void Parser::setup() {
  program.bottomTypes.intType = std::make_shared<Type>(
      BottomType::INT, std::vector<std::shared_ptr<Impl>>{});
  program.bottomTypes.charType = std::make_shared<Type>(
      BottomType::CHAR, std::vector<std::shared_ptr<Impl>>{});
  program.bottomTypes.boolType = std::make_shared<Type>(
      BottomType::BOOL, std::vector<std::shared_ptr<Impl>>{});
  program.bottomTypes.floatType = std::make_shared<Type>(
      BottomType::FLOAT, std::vector<std::shared_ptr<Impl>>{});
  program.bottomTypes.voidType = std::make_shared<Type>(
      BottomType::VOID, std::vector<std::shared_ptr<Impl>>{});
  program.bottomTypes.selfType = std::make_shared<Type>(
      BottomType::SELF, std::vector<std::shared_ptr<Impl>>{});
}
bool Parser::requireNext(TOKEN_TYPE type) {
  Lexer lex = lexer;
  if (lex.next().type != type) {
    std::cerr << "Wrong type!";
    return false;
  }
  curr = lexer.next();
  return true;
}
bool Parser::munch(TOKEN_TYPE type) {
  if (!requireNext(type)) return false;
  curr = lexer.next();
  return true;
}
Environment Parser::parse() {
  curr = lexer.next();
  while (curr.type != TOKEN_TYPE::FILE_END) {
    auto global = globals();
    if (!global) continue;
    if (program.members.find(global->getName()) != program.members.end()) {
      std::cerr << "Redefinition of existing global.";
    }
    program.addMember(global->getName(), std::move(global.value()));
  }
  return program;
}
std::optional<Stmt> Parser::globals() {
  std::optional<Stmt> temp;
  if (curr.type == TOKEN_TYPE::TYPE) {
    temp = typeDef();
  } else if (curr.type == TOKEN_TYPE::LET || curr.type == TOKEN_TYPE::CONST) {
    temp = declarationStmt();
  } else if (curr.type == TOKEN_TYPE::IMPL) {
    temp = implStmt();
  } else if (curr.type == TOKEN_TYPE::CLASS) {
    temp = classStmt();
  } else {
    std::cerr << "Bad Global\n";
  }
  munch(TOKEN_TYPE::SEMI);
  return temp;
}
std::optional<Stmt> Parser::stmt() {
  std::optional<Stmt> temp;
  if (curr.type == TOKEN_TYPE::TYPE) {
    temp = typeDef();
  } else if (curr.type == TOKEN_TYPE::LET || curr.type == TOKEN_TYPE::CONST) {
    temp = declarationStmt();
  } else if (curr.type == TOKEN_TYPE::YIELD) {
    temp = yieldStmt();
  } else if (curr.type == TOKEN_TYPE::RETURN) {
    temp = returnStmt();
  } else if (curr.type == TOKEN_TYPE::IMPL) {
    temp = implStmt();
  } else if (curr.type == TOKEN_TYPE::CLASS) {
    temp = classStmt();
  } else if (curr.type == TOKEN_TYPE::CONTINUE) {
    requireNext(TOKEN_TYPE::CONTINUE);
    temp = Stmt{curr.sourceLocation, nullptr, ContinueStmt{}};
    curr = lexer.next();
  } else {
    temp = exprStmt();
  }
  munch(TOKEN_TYPE::SEMI);
  return temp;
}
std::optional<Stmt> Parser::declarationStmt() {
  Stmt ans{curr.sourceLocation, nullptr, DeclarationStmt{}};
  ans.getDeclarationStmt()->consted = curr.type == TOKEN_TYPE::CONST;
  curr = lexer.next();
  requireNext(TOKEN_TYPE::IDEN);
  ans.getDeclarationStmt()->name = std::string{curr.text};
  if (munch(TOKEN_TYPE::COLON)) {
    ans.type = std::make_shared<Type>(type());
  }
  if (munch(TOKEN_TYPE::EQUALS)) {
    ans.getDeclarationStmt()->val = expr();
  }
  if (isImplClass != state::NORMAL && ans.getDeclarationStmt()->consted &&
      !ans.getDeclarationStmt()->val) {
    std::cerr << "Const must have definition.";
  }
  if (!ans.type && !ans.getDeclarationStmt()->val) {
    std::cerr << "Either type or value must be given for inference.";
  }
  return ans;
}
std::optional<Stmt> Parser::classStmt() {
  state prev = isImplClass;
  isImplClass = state::CLASS;
  Stmt ans =
      Stmt{curr.sourceLocation, program.bottomTypes.voidType, ClassStmt{}};
  munch(TOKEN_TYPE::CLASS);
  ans.getClassStmt()->structType =
      std::make_shared<Type>(Type{StructType({}), {}});
  requireNext(TOKEN_TYPE::IDEN);
  if (program.members.find(std::string{curr.text}) != program.members.end()) {
    std::cerr << "Redeclaration!\n";
  }
  ans.getClassStmt()->name = std::string{curr.text};
  munch(TOKEN_TYPE::LBRACKET);
  while (!munch(TOKEN_TYPE::RBRACKET)) {
    ans.getClassStmt()->parameters.emplace_back(declarationStmt().value());
    ans.getClassStmt()->structType->getStructType()->types.emplace_back(
        AliasType{
            ans.getClassStmt()->parameters.back().getDeclarationStmt()->name,
            ans.getClassStmt()
                ->parameters.back()
                .getDeclarationStmt()
                ->val->type});
  }
  isImplClass = prev;
  return ans;
}
std::optional<Stmt> Parser::implStmt() {
  state prev = isImplClass;
  isImplClass = state::IMPL;
  Stmt ans =
      Stmt{curr.sourceLocation, program.bottomTypes.voidType, ImplStmt{}};
  munch(TOKEN_TYPE::IMPL);
  requireNext(TOKEN_TYPE::IDEN);
  ans.getImplStmt()->name = std::string{curr.text};
  if (munch(TOKEN_TYPE::FOR)) {
    requireNext(TOKEN_TYPE::IDEN);
    ans.getImplStmt()->decorating = std::string{curr.text};
    if (!program.getMember(ans.getImplStmt()->name) ||
        !program.getMember(ans.getImplStmt()->name)->isImplStmt()) {
      std::cerr << "Cannot have implementation before declaration of Impl\n";
    }
    if (!program.getMember(ans.getImplStmt()->decorating)->isClassStmt()) {
      std::cerr << "Can only decorate classes.";
    }
    Stmt* impl = program.getMember(ans.getImplStmt()->name);
    int memberCount = 0;
    munch(TOKEN_TYPE::LBRACKET);
    std::vector<Stmt> stmts;
    while (!munch(TOKEN_TYPE::RBRACKET)) {
      stmts.emplace_back(declarationStmt().value());
      if (stmts.back().getDeclarationStmt()->name !=
          impl->getImplStmt()
              ->parameters[memberCount]
              .getDeclarationStmt()
              ->name) {
        std::cerr << "Invalid impl name.";
        return std::nullopt;
      }

      ++memberCount;
    }
    if (memberCount != impl->getImplStmt()->parameters.size()) {
      std::cerr << "Impl not fully implemented.\n";
      return std::nullopt;
    }
    ans.getImplStmt()->parameters.insert(ans.getImplStmt()->parameters.end(),
                                         std::make_move_iterator(stmts.begin()),
                                         std::make_move_iterator(stmts.end()));
    isImplClass = prev;
    return ans;
  } else {
    ans.getImplStmt()->implType = std::make_shared<Type>(Type{Impl{{}}, {}});
    munch(TOKEN_TYPE::LBRACKET);
    while (!munch(TOKEN_TYPE::RBRACKET)) {
      ans.getImplStmt()->parameters.emplace_back(declarationStmt().value());
      ans.getImplStmt()->implType->getImpl()->includes.emplace_back(AliasType{
          ans.getImplStmt()->parameters.back().getDeclarationStmt()->name,
          ans.getImplStmt()
              ->parameters.back()
              .getDeclarationStmt()
              ->val->type});
    }
  }
  isImplClass = prev;
  return ans;
}
std::optional<Stmt> Parser::yieldStmt() {
  auto location = curr.sourceLocation;
  curr = lexer.next();
  std::unique_ptr<Expr> exp(expr());
  return Stmt{location, exp->type, YieldStmt{std::move(exp)}};
}
std::optional<Stmt> Parser::returnStmt() {
  auto location = curr.sourceLocation;
  curr = lexer.next();
  std::unique_ptr<Expr> exp(expr());
  return Stmt{location, exp->type, ReturnStmt{std::move(exp)}};
}
std::optional<Stmt> Parser::exprStmt() {
  auto location = curr.sourceLocation;
  return Stmt{location, program.bottomTypes.voidType, ExprStmt{expr()}};
}

std::optional<Stmt> Parser::typeDef() {
  auto location = curr.sourceLocation;
  curr = lexer.next();
  requireNext(TOKEN_TYPE::IDEN);
  std::string name{curr.text};
  munch(TOKEN_TYPE::ASSIGN);
  return Stmt{
      location, program.bottomTypes.voidType,
      TypeDef{std::make_shared<Type>(Type{AliasType{name, type()}, {}})}};
}
std::shared_ptr<Type> Parser::productType() {
  switch (curr.type) {
    case TOKEN_TYPE::FN:
      return functionType();
    case TOKEN_TYPE::LIST:
      return listType();
    case TOKEN_TYPE::LEFT_PAREN:
      return tupleType();
    case TOKEN_TYPE::OPTIONAL:
      return optionalType();
    case TOKEN_TYPE::SELF:
      if (isImplClass == state::NORMAL) {
        std::cerr << "Cannot reference self outside of IMPL or Class.\n";
        break;
      }
      return program.getMember("self")->type;
    case TOKEN_TYPE::IDEN:
      return bottomType();
    default:
      std::cerr << "This is not a type";
  }
}
std::shared_ptr<Type> Parser::functionType() {
  munch(TOKEN_TYPE::FN);
  munch(TOKEN_TYPE::LEFT_PAREN);
  std::shared_ptr<Type> ans = std::make_shared<Type>(Type{FunctionType{}, {}});
  while (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
    ans->getFunctionType()->parameters.emplace_back(type());
    if (curr.type != TOKEN_TYPE::RIGHT_PAREN) munch(TOKEN_TYPE::COMMA);
  }
  munch(TOKEN_TYPE::RIGHT_PAREN);
  munch(TOKEN_TYPE::ARROW);
  ans->getFunctionType()->returner = type();
  return ans;
}
std::shared_ptr<Type> Parser::optionalType() {
  munch(TOKEN_TYPE::OPTIONAL);
  munch(TOKEN_TYPE::LSQUARE);
  std::shared_ptr<Type> ans =
      std::make_shared<Type>(Type{OptionalType{type()}, {}});
  munch(TOKEN_TYPE::RSQUARE);
  if (ans->getOptionalType()->optional->isBottomType() &&
      ans->getOptionalType()->optional->getBottomType() == BottomType::VOID) {
    return program.bottomTypes.voidType;
  }
  return ans;
}
std::shared_ptr<Type> Parser::listType() {
  munch(TOKEN_TYPE::LIST);
  munch(TOKEN_TYPE::LSQUARE);
  int size;
  if (curr.type == TOKEN_TYPE::INT) {
    std::from_chars(curr.text.data(), curr.text.data() + curr.text.size(),
                    size);
  } else if (curr.type == TOKEN_TYPE::STAR) {
    size = -1;
  } else {
    std::cerr << "Bad INT\n";
  }
  munch(TOKEN_TYPE::COMMA);
  std::shared_ptr<Type> ans =
      std::make_shared<Type>(Type{ListType{size, type()}, {}});
  munch(TOKEN_TYPE::RSQUARE);
  return ans;
}
std::shared_ptr<Type> Parser::tupleType() {
  curr = lexer.next();
  std::shared_ptr<Type> ans = std::make_shared<Type>(Type{TupleType{}, {}});
  while (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
    ans->getTupleType()->types.emplace_back(std::make_shared<Type>(type()));
    if (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
      munch(TOKEN_TYPE::COMMA);
    }
  }
  munch(TOKEN_TYPE::RIGHT_PAREN);
  return ans;
}
std::shared_ptr<Type> Parser::type() {
  std::shared_ptr<Type> prev = productType();
  if (curr.type != TOKEN_TYPE::BITOR) return prev;
  std::shared_ptr<Type> ans = std::make_shared<Type>(Type{SumType{{prev}}, {}});
  while (curr.type == TOKEN_TYPE::BITOR) {
    curr = lexer.next();
    ans->getSumType()->types.emplace_back(productType());
  }
  return ans;
}
std::shared_ptr<Type> Parser::bottomType() {
  if (curr.text == "int") {
    curr = lexer.next();
    return program.bottomTypes.intType;
  } else if (curr.text == "float") {
    curr = lexer.next();
    return program.bottomTypes.floatType;
  } else if (curr.text == "void") {
    curr = lexer.next();
    return program.bottomTypes.voidType;
  } else if (curr.text == "char") {
    curr = lexer.next();
    return program.bottomTypes.charType;
  } else if (curr.text == "bool") {
    curr = lexer.next();
    return program.bottomTypes.boolType;
  } else {
    std::string typeText{curr.text};
    curr = lexer.next();
    if (program.getMember(typeText)) {
      Stmt* s = program.getMember(typeText);
      if (s->isTypeDef()) {
        return s->getTypeDef()->type;
      } else if (s->isClassStmt()) {
        return s->getClassStmt()->structType;
      } else if (s->isImplStmt()) {
        return s->getImplStmt()->implType;
      } else {
        std::cerr << "Identifier is not of a valid type.\n";
      }
    } else {
      return std::make_shared<Type>(Type{AliasType(typeText, nullptr), {}});
    }
  }
}
std::unique_ptr<Expr> Parser::expr() {
  switch (curr.type) {
    case TOKEN_TYPE::IF:
      return ifExpr();
    case TOKEN_TYPE::FOR:
      return forExpr();
    case TOKEN_TYPE::MATCH:
      return matchExpr();
    case TOKEN_TYPE::WHILE:
      return whileExpr();
    case TOKEN_TYPE::LBRACKET:
      return block();
    case TOKEN_TYPE::FN:
      return functionExpr();
    default:
      return assign();
  }
}
std::unique_ptr<Expr> Parser::forExpr() {
  requireNext(TOKEN_TYPE::FOR);
  std::unique_ptr<Expr> ans =
      std::make_unique<Expr>(curr.sourceLocation, nullptr, ForExpr{});
  curr = lexer.next();
  Environment prev = program;
  program = program.generateInnerEnvironment();
  auto iter = forConditionExpr();
  if (!iter) return nullptr;
  program.addMember(iter->getDeclarationStmt()->name, std::move(iter.value()));
  ans->getForExpr()->body = expr();
  ans->getForExpr()->env = std::make_unique<Environment>(program);
  program = prev;
  return ans;
}
std::optional<Stmt> Parser::forConditionExpr() {
  requireNext(TOKEN_TYPE::IDEN);
  std::string name{curr.text};
  Stmt declaration{curr.sourceLocation, nullptr,
                   DeclarationStmt{false, name, nullptr}};
  munch(TOKEN_TYPE::IN);
  declaration.getDeclarationStmt()->val = expr();
  return declaration;
}
std::unique_ptr<Expr> Parser::ifExpr() {
  curr = lexer.next();
  std::unique_ptr<Expr> exp =
      std::make_unique<Expr>(curr.sourceLocation, nullptr, IfExpr{});
  exp->getIfExpr()->cond = expr();
  exp->getIfExpr()->thenExpr = expr();
  if (curr.type == TOKEN_TYPE::ELSE) {
    curr = lexer.next();
    exp->getIfExpr()->elseExpr = expr();
  }
  return exp;
}
std::unique_ptr<Expr> Parser::whileExpr() {
  curr = lexer.next();
  std::unique_ptr<Expr> exp =
      std::make_unique<Expr>(curr.sourceLocation, nullptr, WhileExpr{});
  exp->getWhileExpr()->cond = expr();
  exp->getWhileExpr()->body = expr();
  return exp;
}
std::unique_ptr<Expr> Parser::block() {
  curr = lexer.next();
  Environment prev = program;
  program = program.generateInnerEnvironment();
  std::unique_ptr<Expr> exp =
      std::make_unique<Expr>(curr.sourceLocation, nullptr, BlockExpr{});
  while (curr.type != TOKEN_TYPE::RBRACKET) {
    exp->getBlockExpr()->stmts.emplace_back(
        std::make_unique<Stmt>(stmt().value()));
    if (exp->getBlockExpr()->stmts.back()->isReturnStmt()) {
      exp->getBlockExpr()->returns = true;
    } else if (exp->getBlockExpr()->stmts.back()->isYieldStmt()) {
      exp->getBlockExpr()->yields = true;
    }
  }
  munch(TOKEN_TYPE::RBRACKET);
  exp->getBlockExpr()->env = std::make_unique<Environment>(program);
  program = prev;
  return exp;
}
std::unique_ptr<Expr> Parser::matchExpr() {
  std::unique_ptr<Expr> ans =
      std::make_unique<Expr>(curr.sourceLocation, nullptr, MatchExpr{});
  munch(TOKEN_TYPE::MATCH);
  ans->getMatchExpr()->cond = expr();
  munch(TOKEN_TYPE::LBRACKET);
  while (!munch(TOKEN_TYPE::RBRACKET)) {
    ans->getMatchExpr()->cases.emplace_back(caseExpr());
  }
  return ans;
}
CaseExpr Parser::caseExpr() {
  munch(TOKEN_TYPE::CASE);
  CaseExpr ans{};
  ans.cond = expr();
  munch(TOKEN_TYPE::ARROW);
  ans.body = expr();
  ans.type = ans.body->type;
  return ans;
}
std::unique_ptr<Expr> Parser::functionExpr() {
  curr = lexer.next();
  Environment prev = program;
  program = program.generateInnerEnvironment();
  std::unique_ptr<Expr> exp =
      std::make_unique<Expr>(curr.sourceLocation, nullptr, FunctionExpr{});
  munch(TOKEN_TYPE::LEFT_PAREN);
  curr = lexer.next();
  std::vector<std::shared_ptr<Type>> types;
  bool inClass = isImplClass == state::CLASS;
  int arity = 0;
  while (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
    do {
      if (inClass && curr.type == TOKEN_TYPE::SELF) {
        inClass = false;
        if (curr.type == TOKEN_TYPE::SELF) {
          types.emplace_back(program.bottomTypes.selfType);
          program.addMember(
              "self", Stmt{curr.sourceLocation, program.bottomTypes.selfType,
                           DeclarationStmt{false, "self", nullptr}});
          ++arity;
        }
      } else {
        inClass = false;
        std::string paramName{curr.text};
        curr = lexer.next();
        munch(TOKEN_TYPE::COLON);
        std::shared_ptr<Type> paramType = type();
        types.emplace_back(paramType);
        program.addMember(paramName,
                          Stmt{curr.sourceLocation, paramType,
                               DeclarationStmt{false, paramName, nullptr}});
        ++arity;
      }
    } while (munch(TOKEN_TYPE::COMMA));
    exp->getFunctionExpr()->arity = arity;
  }
  munch(TOKEN_TYPE::ARROW);
  exp->getFunctionExpr()->parameters = std::make_unique<Environment>(program);
  program = prev;
  exp->getFunctionExpr()->returnType = std::make_shared<Type>(type());
  exp->getFunctionExpr()->action = expr();
  exp->type = std::make_shared<Type>(
      Type{FunctionType{exp->getFunctionExpr()->returnType, types},
           std::vector<std::shared_ptr<Impl>>{}});
  return exp;
}
std::unique_ptr<Expr> Parser::assign() {
  std::unique_ptr<Expr> ans = orExpr();
  if (requireNext(TOKEN_TYPE::ASSIGN)) {
    std::unique_ptr<Expr> assignExpr =
        std::make_unique<Expr>(Expr::makeBinary(curr, ans->type));
    assignExpr->getBinaryExpr()->left = std::move(ans);
    assignExpr->getBinaryExpr()->right = expr();
    ans = std::move(assignExpr);
  }
  return ans;
}
std::unique_ptr<Expr> Parser::rangeExpr() {
  std::unique_ptr<Expr> exp = orExpr();
  if (curr.type == TOKEN_TYPE::RANGE || curr.type == TOKEN_TYPE::INCRANGE) {
    std::unique_ptr<Expr> range =
        std::make_unique<Expr>(Expr::makeBinary(curr, exp->type));
    curr = lexer.next();
    range->getBinaryExpr()->left = std::move(exp);
    range->getBinaryExpr()->right = orExpr();
    exp = std::move(range);
  }
  return exp;
}
std::unique_ptr<Expr> Parser::orExpr() {
  std::unique_ptr<Expr> expr = andExpr();
  while (curr.type == TOKEN_TYPE::OR) {
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, program.bottomTypes.boolType));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->getBinaryExpr()->right = andExpr();
    expr = std::move(binary);
  }
  return expr;
}
std::unique_ptr<Expr> Parser::andExpr() {
  std::unique_ptr<Expr> expr = bitOrExpr();
  while (curr.type == TOKEN_TYPE::AND) {
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, program.bottomTypes.intType));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->getBinaryExpr()->right = bitOrExpr();
    expr = std::move(binary);
  }
  return expr;
}
std::unique_ptr<Expr> Parser::bitOrExpr() {
  std::unique_ptr<Expr> expr = xorExpr();
  while (curr.type == TOKEN_TYPE::BITOR) {
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, program.bottomTypes.intType));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->getBinaryExpr()->right = xorExpr();
    expr = std::move(binary);
  }
  return expr;
}
std::unique_ptr<Expr> Parser::xorExpr() {
  std::unique_ptr<Expr> expr = bitAndExpr();
  while (curr.type == TOKEN_TYPE::XOR) {
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, program.bottomTypes.intType));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->getBinaryExpr()->right = bitAndExpr();
    expr = std::move(binary);
  }
  return expr;
}
std::unique_ptr<Expr> Parser::bitAndExpr() {
  std::unique_ptr<Expr> expr = add();
  while (curr.type == TOKEN_TYPE::BITAND) {
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, program.bottomTypes.intType));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->getBinaryExpr()->right = add();
    expr = std::move(binary);
  }
  return expr;
}
std::unique_ptr<Expr> Parser::equateExpr() {
  std::unique_ptr<Expr> expr = notExpr();
  while (curr.type == TOKEN_TYPE::EQUALS || curr.type == TOKEN_TYPE::NEQUALS) {
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, program.bottomTypes.intType));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->getBinaryExpr()->right = notExpr();
    expr = std::move(binary);
  }
  return expr;
}
std::unique_ptr<Expr> Parser::notExpr() {
  if (curr.type == TOKEN_TYPE::NOT) {
    std::unique_ptr<Expr> expr = std::make_unique<Expr>(
        Expr{curr.sourceLocation, nullptr, PrefixExpr{curr}});
    curr = lexer.next();
    expr->getPrefixExpr()->expr = access();
    expr->type = expr->getPrefixExpr()->expr->type;
    return expr;
  } else {
    return add();
  }
}
std::unique_ptr<Expr> Parser::add() {
  std::unique_ptr<Expr> expr = mult();
  while (curr.type == TOKEN_TYPE::PLUS || curr.type == TOKEN_TYPE::MINUS) {
    std::unique_ptr<Expr> binary =
        std::make_unique<Expr>(Expr::makeBinary(curr, nullptr));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->type = binary->getBinaryExpr()->left->type;
    binary->getBinaryExpr()->right = mult();
    expr = std::move(binary);
  }
  return expr;
}
std::unique_ptr<Expr> Parser::mult() {
  std::unique_ptr<Expr> expr = negate();
  while (curr.type == TOKEN_TYPE::STAR || curr.type == TOKEN_TYPE::SLASH) {
    std::unique_ptr<Expr> binary =
        std::make_unique<Expr>(Expr::makeBinary(curr, nullptr));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->type = binary->getBinaryExpr()->left->type;
    binary->getBinaryExpr()->right = negate();
    expr = std::move(binary);
  }
  return expr;
}
std::unique_ptr<Expr> Parser::negate() {
  if (curr.type == TOKEN_TYPE::MINUS) {
    std::unique_ptr<Expr> expr = std::make_unique<Expr>(
        Expr{curr.sourceLocation, nullptr, PrefixExpr{curr}});
    curr = lexer.next();
    expr->getPrefixExpr()->expr = access();
    expr->type = expr->getPrefixExpr()->expr->type;
    return expr;
  } else {
    return access();
  }
}
std::unique_ptr<Expr> Parser::access() {
  std::unique_ptr<Expr> expr = primary();
  for (;;) {
    if (curr.type == TOKEN_TYPE::LEFT_PAREN) {
      std::unique_ptr<Expr> func =
          std::make_unique<Expr>(curr.sourceLocation, nullptr, CallExpr{});
      func->getCallExpr()->expr = std::move(expr);
      curr = lexer.next();
      if (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
        do {
          func->getCallExpr()->params.emplace_back(this->expr().release());
        } while (curr.type == TOKEN_TYPE::COMMA);
      }
      expr = std::move(func);
      curr = lexer.next();
    } else if (curr.type == TOKEN_TYPE::DOT) {
      requireNext(TOKEN_TYPE::IDEN);
      std::unique_ptr<Expr> getter =
          std::make_unique<Expr>(curr.sourceLocation, nullptr, GetExpr{});
      getter->getGetExpr()->expr = std::move(expr);
      getter->getGetExpr()->name.name = curr.text;
      expr = std::move(getter);
      curr = lexer.next();
    } else {
      break;
    }
  }
  return expr;
}
std::string fixer(const std::string_view orig) {
  const std::string_view no_quotes = orig.substr(1, orig.size() - 1);
  std::string returner;
  for (int i = 0; i < no_quotes.size(); ++i) {
    if (no_quotes[i] != '\\') {
      returner.push_back(orig[i]);
      continue;
    }
    if (no_quotes[i + 1] == 'x') {
      returner.push_back(
          (char)stoi(std::string{no_quotes.substr(i + 2, 2)}, nullptr, 16));
      i += 3;
    } else {
      switch (no_quotes[i + 1]) {
        case 'a':
          returner.push_back('\a');
          break;
        case 'b':
          returner.push_back('\b');
          break;
        case 'f':
          returner.push_back('\f');
          break;
        case 'n':
          returner.push_back('\n');
          break;
        case 'r':
          returner.push_back('\r');
          break;
        case 't':
          returner.push_back('\t');
          break;
        case 'v':
          returner.push_back('\v');
          break;
        case '\'':
          returner.push_back('\'');
          break;
        case '"':
          returner.push_back('\"');
          break;
        case '?':
          returner.push_back('\?');
          break;
        case '\\':
          returner.push_back('\\');
          break;
        default:
          returner.push_back((char)0);
      }
      ++i;
    }
  }
  return returner;
}
std::unique_ptr<Expr> Parser::primary() {
  switch (curr.type) {
    case TOKEN_TYPE::INT: {
      int val{};
      auto result = std::from_chars(curr.text.data(),
                                    curr.text.data() + curr.text.size(), val);
      if (result.ec == std::errc::invalid_argument) {
        std::cerr << "Could not convert to int at: " << curr.sourceLocation.line
                  << ":" << curr.sourceLocation.character << ".\n";
        return {};
      }
      std::unique_ptr<Expr> expr = std::make_unique<Expr>(
          Expr::makeInt(curr, program.bottomTypes.intType, val));
      curr = lexer.next();
      return expr;
    }
    case TOKEN_TYPE::FLOAT: {
      double val{};
      auto result = std::from_chars(curr.text.data(),
                                    curr.text.data() + curr.text.size(), val);
      if (result.ec == std::errc::invalid_argument) {
        std::cerr << "Could not convert to int at: " << curr.sourceLocation.line
                  << ":" << curr.sourceLocation.character << ".\n";
        return {};
      }
      std::unique_ptr<Expr> expr = std::make_unique<Expr>(
          Expr::makeFloat(curr, program.bottomTypes.floatType, val));
      curr = lexer.next();
      return expr;
    }
    case TOKEN_TYPE::LEFT_PAREN: {
      curr = lexer.next();
      auto result = expr();
      if (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
        std::cerr << "Parentheses not closed at: " << curr.sourceLocation.line
                  << ":" << curr.sourceLocation.character << '\n';
        return {};
      }
      curr = lexer.next();
      return result;
    }
    case TOKEN_TYPE::CHAR: {
      if (curr.text.size() == 3) {
        char c = curr.text.at(1);
        std::unique_ptr<Expr> returner = std::make_unique<Expr>(
            curr.sourceLocation, program.bottomTypes.charType, CharExpr(c));
        curr = lexer.next();
        return returner;
      } else if (curr.text.size() == 4) {
        char c;
        switch (curr.text.at(2)) {
          case 'a':
            c = '\a';
            break;
          case 'b':
            c = '\b';
            break;
          case 'f':
            c = '\f';
            break;
          case 'n':
            c = '\n';
            break;
          case 'r':
            c = '\r';
            break;
          case 't':
            c = '\t';
            break;
          case 'v':
            c = '\v';
            break;
          case '\'':
            c = '\'';
            break;
          case '"':
            c = '\"';
            break;
          case '?':
            c = '\?';
            break;
          case '\\':
            c = '\\';
            break;
          default:
            c = 0;
        }
        std::unique_ptr<Expr> returner = std::make_unique<Expr>(
            curr.sourceLocation, program.bottomTypes.charType, CharExpr(c));
        curr = lexer.next();
        return returner;
      } else {
        char c = std::stoi(std::string{curr.text.substr(3, 2)}, 0, 16);
        std::unique_ptr<Expr> returner = std::make_unique<Expr>(
            curr.sourceLocation, program.bottomTypes.charType, CharExpr(c));
        curr = lexer.next();
        return returner;
      }
    }
    case TOKEN_TYPE::STRING: {
      std::string correct = fixer(curr.text);
      std::unique_ptr<Expr> returner = std::make_unique<Expr>(
          curr.sourceLocation,
          std::make_shared<Type>(
              ListType(correct.size(), program.bottomTypes.charType),
              std::vector<std::shared_ptr<Impl>>{}),
          StringExpr(std::string{correct}));
      curr = lexer.next();
      return returner;
    }
    case TOKEN_TYPE::IDEN: {
      std::unique_ptr<Expr> returner = std::make_unique<Expr>(
          curr.sourceLocation, program.bottomTypes.voidType,
          LiteralExpr(curr.text));
      curr = lexer.next();
      return returner;
    }
    case TOKEN_TYPE::TRUE: {
      std::unique_ptr<Expr> exp = std::make_unique<Expr>(
          curr.sourceLocation, program.bottomTypes.boolType, BoolExpr{true});
      curr = lexer.next();
      return exp;
    }
    case TOKEN_TYPE::FALSE: {
      std::unique_ptr<Expr> exp = std::make_unique<Expr>(
          curr.sourceLocation, program.bottomTypes.boolType, BoolExpr{false});
      curr = lexer.next();
      return exp;
    }
    case TOKEN_TYPE::SELF: {
      if (isImplClass != state::NORMAL) {
        std::unique_ptr<Expr> returner = std::make_unique<Expr>(
            curr.sourceLocation, program.bottomTypes.selfType,
            LiteralExpr(curr.text));
        curr = lexer.next();
        return returner;
      } else {
        std::cerr << "SELF cannot exist outside of an Impl or Class\n.";
      }
    }
    default:
      std::cerr << "Invalid token at: " << curr.sourceLocation.line << ":"
                << curr.sourceLocation.character << "!\n";
      return {};
  }
}
