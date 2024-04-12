// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#include "parser.h"

#include <pthread.h>

#include <charconv>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "expr.h"
#include "token.h"
#include "types.h"
void Parser::setup() {
  program.bottomTypes.intType =
      std::make_shared<Type>(BottomType::INT, std::vector<Impl>{});
  program.bottomTypes.charType =
      std::make_shared<Type>(BottomType::CHAR, std::vector<Impl>{});
  program.bottomTypes.boolType =
      std::make_shared<Type>(BottomType::BOOL, std::vector<Impl>{});
  program.bottomTypes.floatType =
      std::make_shared<Type>(BottomType::FLOAT, std::vector<Impl>{});
  program.bottomTypes.voidType =
      std::make_shared<Type>(BottomType::VOID, std::vector<Impl>{});
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
    program.members.emplace(globals());
  }
  return program;
}
Stmt Parser::globals() {
  Stmt temp;
  if (curr.type == TOKEN_TYPE::TYPE) {
    temp = typeDef();
  } else if (curr.type == TOKEN_TYPE::LET || curr.type == TOKEN_TYPE::CONST) {
    return declarationStmt();
  } else if (curr.type == TOKEN_TYPE::IMPL) {
    return implStmt();
  } else if (curr.type == TOKEN_TYPE::CLASS) {
    return classStmt();
  } else {
    std::cerr << "Bad Global\n";
  }
  munch(TOKEN_TYPE::SEMI);
  return temp;
}
Stmt Parser::stmt() {
  Stmt temp;
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
    munch(TOKEN_TYPE::CONTINUE);
    temp = Stmt{nullptr, ContinueStmt{}};
  } else {
    temp = exprStmt();
  }
  munch(TOKEN_TYPE::SEMI);
  return temp;
}
Stmt Parser::declarationStmt() {
  Stmt ans{nullptr, DeclarationStmt{}};
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
  if (!inImplClass && ans.getDeclarationStmt()->consted &&
      !ans.getDeclarationStmt()->val) {
    std::cerr << "Const must have definition.";
  }
  if (!ans.type && !ans.getDeclarationStmt()->val) {
    std::cerr << "Either type or value must be given for inference.";
  }
  return ans;
}
Stmt Parser::classStmt() {
  munch(TOKEN_TYPE::CLASS);
  inImplClass = true;
  Stmt ans = Stmt{program.bottomTypes.voidType, ClassStmt{}};
  requireNext(TOKEN_TYPE::IDEN);
  if (program.members.find(std::string{curr.text}) != program.members.end()) {
    std::cerr << "Redeclaration!\n";
  }
  ans.getClassStmt()->name = std::string{curr.text};
  munch(TOKEN_TYPE::LBRACKET);
  while (!munch(TOKEN_TYPE::RBRACKET)) {
    ans.getClassStmt()->parameters.emplace_back(
        declarationStmt().getDeclarationStmt());
    do {
      ans.getClassStmt()->parameters.emplace_back(
          declarationStmt().getDeclarationStmt());
    } while (munch(TOKEN_TYPE::COMMA));
  }
  inImplClass = false;
  return ans;
}
Stmt Parser::implStmt() {
  inImplClass = true;
  munch(TOKEN_TYPE::IMPL);
  requireNext(TOKEN_TYPE::IDEN);
  Stmt ans = Stmt{program.bottomTypes.voidType, ImplStmt{}};
  ans.getImplStmt()->name = std::string{curr.text};
  if (munch(TOKEN_TYPE::FOR)) {
    requireNext(TOKEN_TYPE::IDEN);
    ans.getImplStmt()->decorating = std::string{curr.text};
  }
  munch(TOKEN_TYPE::LBRACKET);
  while (!munch(TOKEN_TYPE::RBRACKET)) {
    ans.getImplStmt()->parameters.emplace_back(
        declarationStmt().getDeclarationStmt());
    do {
      ans.getImplStmt()->parameters.emplace_back(
          declarationStmt().getDeclarationStmt());
    } while (munch(TOKEN_TYPE::COMMA));
  }
  inImplClass = false;
  return ans;
}
Stmt Parser::yieldStmt() {
  curr = lexer.next();
  std::unique_ptr<Expr> exp(expr());
  return Stmt{exp->type, YieldStmt{std::move(exp)}};
}
Stmt Parser::returnStmt() {
  curr = lexer.next();
  std::unique_ptr<Expr> exp(expr());
  return Stmt{exp->type, ReturnStmt{std::move(exp)}};
}
Stmt Parser::exprStmt() {
  return Stmt{program.bottomTypes.voidType, ExprStmt{expr()}};
}

Stmt Parser::typeDef() {
  curr = lexer.next();
  requireNext(TOKEN_TYPE::IDEN);
  std::string name{curr.text};
  munch(TOKEN_TYPE::ASSIGN);
  Type t = type();
  return Stmt{program.bottomTypes.voidType,
              TypeDef{std::make_shared<AliasType>(name, t)}};
}
Type Parser::productType() {
  switch (curr.type) {
    case TOKEN_TYPE::FN:
      return functionType();
    case TOKEN_TYPE::LIST:
      return listType();
    case TOKEN_TYPE::LEFT_PAREN:
      return tupleType();
    case TOKEN_TYPE::OPTIONAL:
      return optionalType();
    case TOKEN_TYPE::IDEN:
      return bottomType();
    default:
      std::cerr << "This is not a type";
  }
}
Type Parser::functionType() {
  munch(TOKEN_TYPE::FN);
  munch(TOKEN_TYPE::LEFT_PAREN);
  Type ans{FunctionType{}, {}};
  while (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
    ans.getFunctionType()->parameters.emplace_back(
        std::make_shared<Type>(type()));
    if (curr.type != TOKEN_TYPE::RIGHT_PAREN) munch(TOKEN_TYPE::COMMA);
  }
  munch(TOKEN_TYPE::RIGHT_PAREN);
  munch(TOKEN_TYPE::ARROW);
  ans.getFunctionType()->returner = std::make_shared<Type>(type());
  return ans;
}
Type Parser::optionalType() {
  munch(TOKEN_TYPE::OPTIONAL);
  munch(TOKEN_TYPE::LSQUARE);
  Type ans{OptionalType{std::make_shared<Type>(type())}, {}};
  munch(TOKEN_TYPE::RSQUARE);
  if (ans.getOptionalType()->optional->isBottomType() &&
      ans.getOptionalType()->optional->getBottomType() == BottomType::VOID) {
    return {BottomType::VOID, {}};
  }
  return ans;
}
Type Parser::listType() {
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
  Type ans{ListType{size, std::make_shared<Type>(type())}, {}};
  munch(TOKEN_TYPE::RSQUARE);
  return ans;
}
Type Parser::tupleType() {
  curr = lexer.next();
  Type ans{TupleType{}, {}};
  while (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
    ans.getTupleType()->types.emplace_back(std::make_shared<Type>(type()));
    if (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
      munch(TOKEN_TYPE::COMMA);
    }
  }
  munch(TOKEN_TYPE::RIGHT_PAREN);
  return ans;
}
Type Parser::type() {
  Type prev = productType();
  if (curr.type != TOKEN_TYPE::BITOR) return prev;
  Type ans{SumType{{std::make_shared<Type>(prev)}}, {}};
  while (curr.type == TOKEN_TYPE::BITOR) {
    curr = lexer.next();
    Type next = productType();
    ans.getSumType()->types.emplace_back(std::make_shared<Type>(next));
  }
  return ans;
}
Type Parser::bottomType() {
  if (curr.text == "int") {
    Type ans = {BottomType::INT, {}};
    curr = lexer.next();
    return ans;
  } else if (curr.text == "float") {
    Type ans = {BottomType::FLOAT, {}};
    curr = lexer.next();
    return ans;
  } else if (curr.text == "void") {
    Type ans = {BottomType::VOID, {}};
    curr = lexer.next();
    return ans;
  } else if (curr.text == "char") {
    Type ans = {BottomType::CHAR, {}};
    curr = lexer.next();
    return ans;
  } else if (curr.text == "bool") {
    Type ans = {BottomType::BOOL, {}};
    curr = lexer.next();
    return ans;
  } else {
    Type ans = {AliasType{curr.text}, {}};
    curr = lexer.next();
    return ans;
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
  ans->getForExpr()->expr = forConditionExpr();
  ans->getForExpr()->body = expr();
  return ans;
}
ForConditionExpr Parser::forConditionExpr() {
  requireNext(TOKEN_TYPE::IDEN);
  ForConditionExpr ans{};
  ans.var = LiteralExpr(std::string{curr.text});
  munch(TOKEN_TYPE::IN);
  ans.expr = expr();
  return ans;
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
  std::unique_ptr<Expr> exp =
      std::make_unique<Expr>(curr.sourceLocation, nullptr, BlockExpr{});
  while (curr.type != TOKEN_TYPE::RBRACKET) {
    exp->getBlockExpr()->stmts.emplace_back(std::make_unique<Stmt>(stmt()));
    if (exp->getBlockExpr()->stmts.back()->isReturnStmt()) {
      exp->getBlockExpr()->returns = true;
    } else if (exp->getBlockExpr()->stmts.back()->isYieldStmt()) {
      exp->getBlockExpr()->yields = true;
    }
  }
  munch(TOKEN_TYPE::RBRACKET);
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
  std::unique_ptr<Expr> exp =
      std::make_unique<Expr>(curr.sourceLocation, nullptr, FunctionExpr{});
  munch(TOKEN_TYPE::LEFT_PAREN);
  curr = lexer.next();
  std::vector<std::shared_ptr<Type>> types;
  while (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
    do {
      std::shared_ptr<Type> paramType = std::make_shared<Type>(type());
      std::string paramName{curr.text};
      curr = lexer.next();
      types.emplace_back(paramType);
      exp->getFunctionExpr()->parameters.emplace_back(paramType, paramName);
    } while (munch(TOKEN_TYPE::COMMA));
    exp->getFunctionExpr()->arity = exp->getFunctionExpr()->parameters.size();
  }
  munch(TOKEN_TYPE::ARROW);
  exp->getFunctionExpr()->returnType = std::make_shared<Type>(type());
  exp->getFunctionExpr()->action = expr();
  exp->type = std::make_shared<Type>(
      Type{FunctionType{exp->getFunctionExpr()->returnType, types},
           std::vector<Impl>{}});
  return exp;
}
std::unique_ptr<Expr> Parser::assign() {
  std::unique_ptr<Expr> ans = orExpr();
  if (requireNext(TOKEN_TYPE::EQUALS)) {
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
  std::unique_ptr<Expr> expr = prefix();
  while (curr.type == TOKEN_TYPE::STAR || curr.type == TOKEN_TYPE::SLASH) {
    std::unique_ptr<Expr> binary =
        std::make_unique<Expr>(Expr::makeBinary(curr, nullptr));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->type = binary->getBinaryExpr()->left->type;
    binary->getBinaryExpr()->right = prefix();
    expr = std::move(binary);
  }
  return expr;
}
std::unique_ptr<Expr> Parser::prefix() {
  if (curr.type == TOKEN_TYPE::MINUS) {
    std::unique_ptr<Expr> expr =
        std::make_unique<Expr>(Expr::makeBinary(curr, nullptr));
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
              std::vector<Impl>{}),
          StringExpr(std::string{correct}));
      curr = lexer.next();
      return returner;
    }
    case TOKEN_TYPE::IDEN: {
      std::unique_ptr<Expr> returner = std::make_unique<Expr>(
          curr.sourceLocation, Type{BottomType::VOID, {}}.clone(),
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
    default:
      std::cerr << "Invalid token at: " << curr.sourceLocation.line << ":"
                << curr.sourceLocation.character << "!\n";
      return {};
  }
}
