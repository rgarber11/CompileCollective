// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#include "parser.h"

#include <charconv>
#include <iostream>
#include <memory>
#include <string_view>
#include <utility>

#include "expr.h"
#include "token.h"
#include "types.h"
bool Parser::munch(TOKEN_TYPE type) {
  Lexer lex = lexer;
  if (lex.next().type != type) {
    std::cerr << "Wrong type!";
    return false;
  }
  curr = lexer.next();
  return true;
}
Environment Parser::parse() {
  curr = lexer.next();
  if (curr.type == TOKEN_TYPE::TYPE) {
    program.types.emplace_back(typeDef());
    program.mapper[program.types.back().alias] =
        std::make_pair(false, program.types.size() - 1);
  } else {
    program.globals.emplace_back(stmt());
  }
  return program;
}
Stmt Parser::stmt() {
  if (curr.type == TOKEN_TYPE::LET || curr.type == TOKEN_TYPE::CONST) {
    return declarationStmt();
  } else if (curr.type == TOKEN_TYPE::YIELD) {
    return yieldStmt();
  } else if (curr.type == TOKEN_TYPE::RETURN) {
    return returnStmt();
  } else if (curr.type == TOKEN_TYPE::FN) {
    return functionStmt();
  } else if (curr.type == TOKEN_TYPE::IMPL) {
    return implStmt();
  } else if (curr.type == TOKEN_TYPE::CLASS) {
    return classStmt();
  } else {
    return exprStmt();
  }
}
AliasType Parser::typeDef() {
  curr = lexer.next();
  munch(TOKEN_TYPE::IDEN);
  std::string name{curr.text};
  munch(TOKEN_TYPE::ASSIGN);
  Type t = type();
  munch(TOKEN_TYPE::SEMI);
  return {name, std::make_unique<Type>(t)};
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
    ans.getFunctionType()->parameters.emplace_back(type());
    if (curr.type != TOKEN_TYPE::RIGHT_PAREN) munch(TOKEN_TYPE::COMMA);
  }
  munch(TOKEN_TYPE::ARROW);
  ans.getFunctionType()->returner = std::make_unique<Type>(type());
  return ans;
}
Type Parser::optionalType() {
  munch(TOKEN_TYPE::OPTIONAL);
  munch(TOKEN_TYPE::LSQUARE);
  Type ans{OptionalType{type().clone()}, {}};
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
  Type ans{ListType{type().clone()}, {}};
  munch(TOKEN_TYPE::RSQUARE);
  return ans;
}
Type Parser::tupleType() {
  curr = lexer.next();
  Type ans{TupleType{}, {}};
  while (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
    ans.getTupleType()->types.emplace_back(type());
    if (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
      munch(TOKEN_TYPE::COMMA);
    }
  }
}
Type Parser::type() {
  Type prev = productType();
  if (curr.type != TOKEN_TYPE::BITOR) return prev;
  Type ans{SumType{{prev}}, {}};
  while (curr.type == TOKEN_TYPE::BITOR) {
    curr = lexer.next();
    Type next = productType();
    ans.getSumType()->types.emplace_back(next);
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
    default:
      return orExpr();
  }
}
std::unique_ptr<Expr> Parser::orExpr() {
  std::unique_ptr<Expr> expr = andExpr();
  while (curr.type == TOKEN_TYPE::OR) {
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, Type{BottomType::BOOL, std::vector<Impl>{}}));
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
        Expr::makeBinary(curr, Type{BottomType::BOOL, std::vector<Impl>{}}));
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
        Expr::makeBinary(curr, Type{BottomType::INT, std::vector<Impl>{}}));
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
        Expr::makeBinary(curr, Type{BottomType::INT, std::vector<Impl>{}}));
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
        Expr::makeBinary(curr, Type{BottomType::INT, std::vector<Impl>{}}));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->getBinaryExpr()->right = addExpr();
    expr = std::move(binary);
  }
  return expr;
}
std::unique_ptr<Expr> Parser::add() {
  std::unique_ptr<Expr> expr = mult();
  while (curr.type == TOKEN_TYPE::PLUS || curr.type == TOKEN_TYPE::MINUS) {
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, Type{BottomType::INT, std::vector<Impl>{}}));
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
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, Type{BottomType::INT, std::vector<Impl>{}}));
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
    std::unique_ptr<Expr> expr = std::make_unique<Expr>(
        Expr::makePrefix(curr, Type{BottomType::INT, std::vector<Impl>{}}));
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
      std::unique_ptr<Expr> func = std::make_unique<Expr>(
          curr.sourceLocation, Type{BottomType::VOID, {}}.clone(), CallExpr{});
      func->getCallExpr()->expr = std::move(expr);
      curr = lexer.next();
      if (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
        do {
          func->getCallExpr()->params.emplace_back(*this->expr().release());
        } while (curr.type == TOKEN_TYPE::COMMA);
      }
      expr = std::move(func);
    } else if (curr.type == TOKEN_TYPE::DOT) {
      munch(TOKEN_TYPE::IDEN);
      std::unique_ptr<Expr> getter = std::make_unique<Expr>(
          curr.sourceLocation, Type{BottomType::VOID, {}}.clone(), GetExpr{});
      getter->getGetExpr()->expr = std::move(expr);
      getter->getGetExpr()->name.name = curr.text;
      expr = std::move(getter);
      curr = lexer.next();
    } else {
      break;
    }
  }
}
std::string fixer(const std::string_view orig) {
  std::string returner;
  for (int i = 1; i < orig.size() - 1; ++i) {
    if (orig[i] != '\\') {
      returner.push_back(orig[i]);
      continue;
    }
    if (orig[i + 1] == 'x') {
      returner.push_back((char)stoi(std::string{orig.substr(i + 2, 2)}, 0, 16));
    } else {
      switch (orig[i + 1]) {
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
      std::unique_ptr<Expr> expr =
          std::make_unique<Expr>(Expr::makeInt(curr, val));
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
      std::unique_ptr<Expr> expr =
          std::make_unique<Expr>(Expr::makeFloat(curr, val));
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
            curr.sourceLocation, Type{BottomType::VOID, {}}.clone(),
            CharExpr(c));
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
            curr.sourceLocation, Type{BottomType::VOID, {}}.clone(),
            CharExpr(c));
        curr = lexer.next();
        return returner;
      } else {
        char c = std::stoi(std::string{curr.text.substr(3, 2)}, 0, 16);
        std::unique_ptr<Expr> returner = std::make_unique<Expr>(
            curr.sourceLocation, Type{BottomType::VOID, {}}.clone(),
            CharExpr(c));
        curr = lexer.next();
        return returner;
      }
    }
    case TOKEN_TYPE::STRING: {
      std::string correct = fixer(curr.text);
      std::unique_ptr<Expr> returner = std::make_unique<Expr>(
          curr.sourceLocation, Type{BottomType::VOID, {}}.clone(),
          StringExpr(correct));
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
    default:
      std::cerr << "Invalid token at: " << curr.sourceLocation.line << ":"
                << curr.sourceLocation.character << "!\n";
      return {};
  }
}
