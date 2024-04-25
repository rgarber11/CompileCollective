// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#include "parser.h"


#include <charconv>
#include <iostream>
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
// Set up the parser
void Parser::setup() {
  program = std::make_unique<Environment>();
  // Pointers to each bottom type
  program->bottomTypes.intType = std::make_shared<Type>(
      BottomType::INT, std::vector<std::shared_ptr<Impl>>{});
  program->bottomTypes.charType = std::make_shared<Type>(
      BottomType::CHAR, std::vector<std::shared_ptr<Impl>>{});
  program->bottomTypes.boolType = std::make_shared<Type>(
      BottomType::BOOL, std::vector<std::shared_ptr<Impl>>{});
  program->bottomTypes.floatType = std::make_shared<Type>(
      BottomType::FLOAT, std::vector<std::shared_ptr<Impl>>{});
  program->bottomTypes.voidType = std::make_shared<Type>(
      BottomType::VOID, std::vector<std::shared_ptr<Impl>>{});
  program->bottomTypes.selfType = std::make_shared<Type>(
      BottomType::SELF, std::vector<std::shared_ptr<Impl>>{});
}
// Returns true if next token is of a certain type
bool Parser::requireNext(TOKEN_TYPE type) {
  Lexer lex = lexer;
  if (lex.next().type != type) {
    return false;
  }
  curr = lexer.next();
  return true;
}
// Require token to be of a certain type, then move on to next token
bool Parser::munch(TOKEN_TYPE type) {
  if (!requireNext(type)) return false;
  curr = lexer.next();
  return true;
}
bool Parser::eatCurr(TOKEN_TYPE type) {
  if(curr.type != type) return false;
  curr = lexer.next();
  return true;
}
// Parse based on type, and return environment
std::unique_ptr<Environment> Parser::parse(Parser::parser is) {
  curr = lexer.next();
  // Handle a program parser, until file end
  if (is == parser::PROGRAM) {
    while (curr.type != TOKEN_TYPE::FILE_END) {
      // Ensure all global statements are valid, and add them to the environment
      auto global = globals();
      if (!global) continue;
      auto nameOfGlobal = global->getName();
      if (program->members.find(nameOfGlobal) != program->members.end()) {
        std::cerr << "Redefinition of existing global.";
      }
      program->addMember(nameOfGlobal, std::move(global.value()));
    }
    return std::move(program);
    // Handle type parser, add to program
  } else if (is == parser::TYPE) {
    program->addMember("$TypeCheckerType",
                      Stmt{curr.sourceLocation, program->bottomTypes.voidType,
                           TypeDef{type()}});
    return std::move(program);
    // Handle expr parser, add to program
  } else {
    program->addMember("$TypeCheckerExpr",
                      Stmt{curr.sourceLocation, program->bottomTypes.voidType,
                           ExprStmt{expr()}});
    return std::move(program);
  }
}
// Return the next global statement
std::optional<Stmt> Parser::globals() {
  std::optional<Stmt> temp;
  // Ensure global statements are valid
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
  // Ensure statement ends in a semicolon;
  if(curr.type == TOKEN_TYPE::SEMI) {
    curr = lexer.next();
  } else {
    std::cerr << "Requires semicolon.";
    exit(-1);
  }
  return temp;
}
// Return the next statement
std::optional<Stmt> Parser::stmt() {
  std::optional<Stmt> temp;
  // Ensure statements are valid
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
  } else if (curr.type == TOKEN_TYPE::CONTINUE && inLoop) {
    requireNext(TOKEN_TYPE::CONTINUE);
    temp = Stmt{curr.sourceLocation, nullptr, ContinueStmt{}};
    curr = lexer.next();
  } else {
    temp = exprStmt();
  }
  // Ensure statement ends in a semicolon
 if(curr.type != TOKEN_TYPE::SEMI) {
   std::cerr << "Statements must end in a semicolon.";
 }
 curr = lexer.next();
  return temp;
}
// Declaration statement - declare a variable
std::optional<Stmt> Parser::declarationStmt() {
  Stmt ans{curr.sourceLocation, nullptr, DeclarationStmt{}};
  ans.getDeclarationStmt()->consted = curr.type == TOKEN_TYPE::CONST;
  curr = lexer.next();
  // Must start with identifier
  requireNext(TOKEN_TYPE::IDEN);
  ans.getDeclarationStmt()->name = std::string{curr.text};
  curr = lexer.next();
  // Declare with : or =
  if (curr.type == (TOKEN_TYPE::COLON)) {
    curr = lexer.next();
    ans.type = type();
  }
  if (curr.type == TOKEN_TYPE::ASSIGN) {
    curr = lexer.next();
    ans.getDeclarationStmt()->val = expr();
  }
  // Ensure const declarations include a value
  if (isImplClass != state::NORMAL && ans.getDeclarationStmt()->consted &&
      !ans.getDeclarationStmt()->val) {
    std::cerr << "Const must have definition.";
  }
  // Ensure statement has at least a type or a value
  if (!ans.type && !ans.getDeclarationStmt()->val) {
    std::cerr << "Either type or value must be given for inference.";
  }
  if(ans.getDeclarationStmt()->val && !ans.type) {
    ans.type = ans.getDeclarationStmt()->val->type;
  }
  if(isImplClass != state::NORMAL && !ans.type) {
    std::cerr << "Class definitions require types.";
  }
  if(ans.getDeclarationStmt()->val && ans.getDeclarationStmt()->val->isFunctionExpr()) {
    ans.getDeclarationStmt()->val->getFunctionExpr()->name = ans.getName();
  }
  return ans;
}
// Class statement - declare a class
std::optional<Stmt> Parser::classStmt() {
  state prev = isImplClass;
  isImplClass = state::CLASS;
  Stmt ans =
      Stmt{curr.sourceLocation, program->bottomTypes.voidType, ClassStmt{}};
  // Begins with class
  ans.getClassStmt()->structType =
      std::make_shared<Type>(Type{StructType({}), {}});
  // Class name
  requireNext(TOKEN_TYPE::IDEN);
  // Disallow redeclaration
  if (program->members.find(std::string{curr.text}) != program->members.end()) {
    std::cerr << "Redeclaration!\n";
  }
  ans.getClassStmt()->name = std::string{curr.text};
  // Define within brackets
  munch(TOKEN_TYPE::LBRACKET);
  while (!munch(TOKEN_TYPE::RBRACKET)) {
    // Parameters are declaration values
    ans.getClassStmt()->parameters.emplace_back(declarationStmt().value());
    if(curr.type != TOKEN_TYPE::SEMI) {std::cerr << "semicolon required after declaration.";}
    curr = lexer.next();
    // Store types
    ans.getClassStmt()->structType->getStructType()->types.emplace_back(

            ans.getClassStmt()->parameters.back().getDeclarationStmt()->name,
            ans.getClassStmt()
                ->parameters.back()
                .getDeclarationStmt()
                ->val->type);
  }
  isImplClass = prev;
  return ans;
}
// Impl statement
std::optional<Stmt> Parser::implStmt() {
  state prev = isImplClass;
  isImplClass = state::IMPL;
  Stmt ans =
      Stmt{curr.sourceLocation, program->bottomTypes.voidType, ImplStmt{}};
  // Begins with impl then an identifier
  munch(TOKEN_TYPE::IMPL);
  requireNext(TOKEN_TYPE::IDEN);
  ans.getImplStmt()->name = std::string{curr.text};
  // For, ensure impl is declared before implementation and only classes are
  // decorated
  if (munch(TOKEN_TYPE::FOR)) {
    requireNext(TOKEN_TYPE::IDEN);
    ans.getImplStmt()->decorating = std::string{curr.text};
    if (!program->getMember(ans.getImplStmt()->name) ||
        !program->getMember(ans.getImplStmt()->name)->isImplStmt()) {
      std::cerr << "Cannot have implementation before declaration of Impl\n";
    }
    if (!program->getMember(ans.getImplStmt()->decorating)->isClassStmt()) {
      std::cerr << "Can only decorate classes.";
    }
    Stmt* impl = program->getMember(ans.getImplStmt()->name);
    int memberCount = 0;
    // Statements within brackets
    munch(TOKEN_TYPE::LBRACKET);
    std::vector<Stmt> stmts;
    while (!munch(TOKEN_TYPE::RBRACKET)) {
      stmts.emplace_back(declarationStmt().value());
      // Ensure impl names are valid
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
    // Ensure full implementation
    if (memberCount != impl->getImplStmt()->parameters.size()) {
      std::cerr << "Impl not fully implemented.\n";
      return std::nullopt;
    }
    // Insert parameters
    ans.getImplStmt()->parameters.insert(ans.getImplStmt()->parameters.end(),
                                         std::make_move_iterator(stmts.begin()),
                                         std::make_move_iterator(stmts.end()));
    isImplClass = prev;
    return ans;
  } else {
    ans.getImplStmt()->implType = std::make_shared<Type>(Type{Impl{{}}, {}});
    // Within brackets
    munch(TOKEN_TYPE::LBRACKET);
    while (!munch(TOKEN_TYPE::RBRACKET)) {
      // Store necessary data
      ans.getImplStmt()->parameters.emplace_back(declarationStmt().value());
      ans.getImplStmt()->implType->getImpl()->includes.emplace_back(
          ans.getImplStmt()->parameters.back().getDeclarationStmt()->name,
          ans.getImplStmt()
              ->parameters.back()
              .getDeclarationStmt()
              ->val->type);
    }
  }
  isImplClass = prev;
  return ans;
}
// Yield statement
std::optional<Stmt> Parser::yieldStmt() {
  auto location = curr.sourceLocation;
  curr = lexer.next();
  if (!inBlock) return std::nullopt;
  std::unique_ptr<Expr> exp(expr());
  return Stmt{location, exp->type, YieldStmt{std::move(exp)}};
}
// Return statement
std::optional<Stmt> Parser::returnStmt() {
  auto location = curr.sourceLocation;
  curr = lexer.next();
  std::unique_ptr<Expr> exp(expr());
  return Stmt{location, exp->type, ReturnStmt{std::move(exp)}};
}
// Expression statement
std::optional<Stmt> Parser::exprStmt() {
  auto location = curr.sourceLocation;
  return Stmt{location, program->bottomTypes.voidType, ExprStmt{expr()}};
}

// Type definition
std::optional<Stmt> Parser::typeDef() {
  auto location = curr.sourceLocation;
  curr = lexer.next();
  // Identifier = ...
  requireNext(TOKEN_TYPE::IDEN);
  std::string name{curr.text};
  munch(TOKEN_TYPE::ASSIGN);
  return Stmt{
      location, program->bottomTypes.voidType,
      TypeDef{std::make_shared<Type>(Type{AliasType{name, type()}, {}})}};
}
// Product types
std::shared_ptr<Type> Parser::productType() {
  // Return types based on token
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
      // Ensure self is only referenced in the proper places
      if (isImplClass == state::NORMAL) {
        std::cerr << "Cannot reference self outside of IMPL or Class.\n";
        break;
      }
      return program->bottomTypes.selfType;
    case TOKEN_TYPE::IDEN:
      return bottomType();
    default:
      std::cerr << "This is not a type";
  }
}
// Function type
std::shared_ptr<Type> Parser::functionType() {
  // fn (...
  munch(TOKEN_TYPE::FN);
  munch(TOKEN_TYPE::LEFT_PAREN);
  std::shared_ptr<Type> ans = std::make_shared<Type>(Type{FunctionType{}, {}});
  // Between parentheses, add parameters separated by commas
  while (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
    ans->getFunctionType()->parameters.emplace_back(type());
    if (curr.type != TOKEN_TYPE::RIGHT_PAREN) munch(TOKEN_TYPE::COMMA);
  }
  // ...)->
  munch(TOKEN_TYPE::RIGHT_PAREN);
  munch(TOKEN_TYPE::ARROW);
  ans->getFunctionType()->returner = type();
  return ans;
}
// Optional type
std::shared_ptr<Type> Parser::optionalType() {
  // optional [...]
  munch(TOKEN_TYPE::OPTIONAL);
  munch(TOKEN_TYPE::LSQUARE);
  std::shared_ptr<Type> ans =
      std::make_shared<Type>(Type{OptionalType{type()}, {}});
  eatCurr(TOKEN_TYPE::RSQUARE);
  // Return void type if necessary
  if (ans->getOptionalType()->optional->isBottomType() &&
      ans->getOptionalType()->optional->getBottomType() == BottomType::VOID) {
    return program->bottomTypes.voidType;
  }
  return ans;
}
// List type
std::shared_ptr<Type> Parser::listType() {
  // list [...]
  munch(TOKEN_TYPE::LIST);
  munch(TOKEN_TYPE::LSQUARE);
  int listSize = -2;
  // Integer (or *),...
  if (curr.type == TOKEN_TYPE::INT) {
    std::from_chars(curr.text.data(), curr.text.data() + curr.text.size(),
                    listSize);
  } else if (curr.type == TOKEN_TYPE::STAR) {
    listSize = -1;
  } else {
    std::cerr << "Bad INT\n";
  }
  munch(TOKEN_TYPE::COMMA);
  std::shared_ptr<Type> ans =
      std::make_shared<Type>(Type{ListType{listSize, std::move(type())}, {}});
  eatCurr(TOKEN_TYPE::RSQUARE);
  return ans;
}
// Tuple type
std::shared_ptr<Type> Parser::tupleType() {
  curr = lexer.next();
  std::shared_ptr<Type> ans = std::make_shared<Type>(Type{TupleType{}, {}});
  // Between parens, add types separated by commas
  while (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
    ans->getTupleType()->types.emplace_back(type());
    if (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
      eatCurr(TOKEN_TYPE::COMMA);
    }
  }
  eatCurr(TOKEN_TYPE::RIGHT_PAREN);
  return ans;
}
// Type type
std::shared_ptr<Type> Parser::type() {
  // Sum and product types
  std::shared_ptr<Type> prev = productType();
  if (curr.type != TOKEN_TYPE::BITOR) return prev;
  std::shared_ptr<Type> ans = std::make_shared<Type>(Type{SumType{{prev}}, {}});
  while (curr.type == TOKEN_TYPE::BITOR) {
    curr = lexer.next();
    ans->getSumType()->types.emplace_back(productType());
  }
  return ans;
}
// Primary (bottom) types
std::shared_ptr<Type> Parser::bottomType() {
  if (curr.text == "int") {
    curr = lexer.next();
    return program->bottomTypes.intType;
  } else if (curr.text == "float") {
    curr = lexer.next();
    return program->bottomTypes.floatType;
  } else if (curr.text == "void") {
    curr = lexer.next();
    return program->bottomTypes.voidType;
  } else if (curr.text == "char") {
    curr = lexer.next();
    return program->bottomTypes.charType;
  } else if (curr.text == "bool") {
    curr = lexer.next();
    return program->bottomTypes.boolType;
  } else {
    // Identifier, ensure proper type
    std::string typeText{curr.text};
    curr = lexer.next();
    if (program->getMember(typeText)) {
      Stmt* s = program->getMember(typeText);
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
// Expressions
std::unique_ptr<Expr> Parser::expr() {
  // Assignment by default
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
// For expression
std::unique_ptr<Expr> Parser::forExpr() {
  // for keyword, inner environment, condition, statements...
  requireNext(TOKEN_TYPE::FOR);
  std::unique_ptr<Expr> ans =
      std::make_unique<Expr>(curr.sourceLocation, nullptr, ForExpr{});
  bool storage = inLoop;
  inLoop = true;
  curr = lexer.next();
  auto prev = std::move(program);
  program = prev->generateInnerEnvironment();
  auto iter = forConditionExpr();
  if (!iter) return nullptr;
  program->addMember(iter->getDeclarationStmt()->name, std::move(iter.value()));
  ans->getForExpr()->body = expr();
  ans->getForExpr()->env = std::move(program);
  program = std::move(prev);
  inLoop = storage;
  return ans;
}
// For condition expression
std::optional<Stmt> Parser::forConditionExpr() {
  // Identifier in...
  requireNext(TOKEN_TYPE::IDEN);
  std::string name{curr.text};
  Stmt declaration{curr.sourceLocation, nullptr,
                   DeclarationStmt{false, name, nullptr}};
  munch(TOKEN_TYPE::IN);
  declaration.getDeclarationStmt()->val = expr();
  return declaration;
}
// If expression
std::unique_ptr<Expr> Parser::ifExpr() {
  curr = lexer.next();
  std::unique_ptr<Expr> exp =
      std::make_unique<Expr>(curr.sourceLocation, nullptr, IfExpr{});
  // If, condition, then, else
  exp->getIfExpr()->cond = expr();
  exp->getIfExpr()->thenExpr = expr();
  if (curr.type == TOKEN_TYPE::ELSE) {
    curr = lexer.next();
    exp->getIfExpr()->elseExpr = expr();
  }
  return exp;
}
// While expression
std::unique_ptr<Expr> Parser::whileExpr() {
  curr = lexer.next();
  bool storage = inLoop;
  inLoop = true;
  std::unique_ptr<Expr> exp =
      std::make_unique<Expr>(curr.sourceLocation, nullptr, WhileExpr{});
  // Condition and body
  exp->getWhileExpr()->cond = expr();
  exp->getWhileExpr()->body = expr();
  inLoop = storage;
  return exp;
}
// Block of code, with environment, between brackets
std::unique_ptr<Expr> Parser::block() {
  curr = lexer.next();
  std::unique_ptr<Environment> prev = std::move(program);
  program = prev->generateInnerEnvironment();
  bool storage = inBlock;
  inBlock = true;
  std::unique_ptr<Expr> exp =
      std::make_unique<Expr>(curr.sourceLocation, nullptr, BlockExpr{});
  while (curr.type != TOKEN_TYPE::RBRACKET) {
    // Add statements, report if returns or yields
    exp->getBlockExpr()->stmts.emplace_back(
        std::make_unique<Stmt>(stmt().value()));
    if (exp->getBlockExpr()->stmts.back()->isReturnStmt()) {
      exp->getBlockExpr()->returns = true;
    } else if (exp->getBlockExpr()->stmts.back()->isYieldStmt()) {
      exp->getBlockExpr()->yields = true;
    }
  }
  if(curr.type != TOKEN_TYPE::RBRACKET) {
    std::cerr << "Blocks need to be close\n";
  }
  curr = lexer.next();
  exp->getBlockExpr()->env = std::move(program);
  inBlock = storage;
  program = std::move(prev);
  return exp;
}
// Match expression
std::unique_ptr<Expr> Parser::matchExpr() {
  std::unique_ptr<Expr> ans =
      std::make_unique<Expr>(curr.sourceLocation, nullptr, MatchExpr{});
  // match, condition, cases between brackets
  curr = lexer.next();
  ans->getMatchExpr()->cond = expr();
  if (curr.type != TOKEN_TYPE::LBRACKET) {
    std::cerr << "Match Expression needs Cases.";
  }
  curr = lexer.next();
  while (curr.type != (TOKEN_TYPE::RBRACKET)) {
    ans->getMatchExpr()->cases.emplace_back(caseExpr());
  }
  curr = lexer.next();
  return ans;
}
// Case expression
CaseExpr Parser::caseExpr() {
  // case condition -> body
  if(curr.type != TOKEN_TYPE::CASE) {
    std::cerr << "Match expressions can only contain cases.\n";
  }
  curr = lexer.next();
  CaseExpr ans{};
  ans.cond = "";
  while (!eatCurr(TOKEN_TYPE::ARROW)) {
    std::get<std::string>(ans.cond).append(curr.text);
    curr = lexer.next();
  }
  ans.body = expr();
  ans.type = ans.body->type;
  return ans;
}
// Function expression
std::unique_ptr<Expr> Parser::functionExpr() {
  curr = lexer.next();
  // Has own environment
  std::unique_ptr<Environment> prev = std::move(program);
  program = prev->generateInnerEnvironment();
  std::unique_ptr<Expr> exp =
      std::make_unique<Expr>(curr.sourceLocation, nullptr, FunctionExpr{});
  munch(TOKEN_TYPE::LEFT_PAREN);
  curr = lexer.next();
  std::vector<std::shared_ptr<Type>> types;
  bool inClass = isImplClass == state::CLASS;
  int arity = 0;
  // Add parameters between parens
  while (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
    do {
      // Deal with self type
      if (inClass && curr.type == TOKEN_TYPE::SELF) {
        inClass = false;
        if (curr.type == TOKEN_TYPE::SELF) {
          types.emplace_back(program->bottomTypes.selfType);
          program->addMember(
              "self", Stmt{curr.sourceLocation, program->bottomTypes.selfType,
                           DeclarationStmt{false, "self", nullptr}});
          ++arity;
        }
      } else {
        inClass = false;
        std::string paramName{curr.text};
        munch(TOKEN_TYPE::COLON);
        std::shared_ptr<Type> paramType = type();
        types.emplace_back(paramType);
        program->addMember(paramName,
                          Stmt{curr.sourceLocation, paramType,
                               DeclarationStmt{false, paramName, nullptr}});
        ++arity;
      }
    } while (eatCurr(TOKEN_TYPE::COMMA));
    exp->getFunctionExpr()->arity = arity;
  }
  munch(TOKEN_TYPE::ARROW);
  // Function has parameters, return type, and action
  exp->getFunctionExpr()->returnType = type();
  exp->getFunctionExpr()->action = expr();
  exp->getFunctionExpr()->parameters = std::move(program);
  program = std::move(prev);
  exp->type = std::make_shared<Type>(
      Type{FunctionType{exp->getFunctionExpr()->returnType, types},
           std::vector<std::shared_ptr<Impl>>{}});
  return exp;
}
// Assignment (operation priority included, in the subsequent operations)
std::unique_ptr<Expr> Parser::assign() {
  std::unique_ptr<Expr> ans = rangeExpr();
  // Create binary expression if next token is ASSIGN
  if (curr.type == (TOKEN_TYPE::ASSIGN)) {
    std::unique_ptr<Expr> assignExpr =
        std::make_unique<Expr>(Expr::makeBinary(curr, ans->type));
    assignExpr->getBinaryExpr()->left = std::move(ans);
    curr = lexer.next();
    assignExpr->getBinaryExpr()->right = expr();
    ans = std::move(assignExpr);
  }
  return ans;
}
// Range expression
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
// Or expression
std::unique_ptr<Expr> Parser::orExpr() {
  std::unique_ptr<Expr> expr = andExpr();
  // Boolean types
  while (curr.type == TOKEN_TYPE::OR) {
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, program->bottomTypes.boolType));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->getBinaryExpr()->right = andExpr();
    expr = std::move(binary);
  }
  return expr;
}
// And expression
std::unique_ptr<Expr> Parser::andExpr() {
  std::unique_ptr<Expr> expr = bitOrExpr();
  while (curr.type == TOKEN_TYPE::AND) {
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, program->bottomTypes.intType));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->getBinaryExpr()->right = bitOrExpr();
    expr = std::move(binary);
  }
  return expr;
}
// Bit or expression
std::unique_ptr<Expr> Parser::bitOrExpr() {
  std::unique_ptr<Expr> expr = xorExpr();
  while (curr.type == TOKEN_TYPE::BITOR) {
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, program->bottomTypes.intType));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->getBinaryExpr()->right = xorExpr();
    expr = std::move(binary);
  }
  return expr;
}
// Exclusive or expression
std::unique_ptr<Expr> Parser::xorExpr() {
  std::unique_ptr<Expr> expr = bitAndExpr();
  while (curr.type == TOKEN_TYPE::XOR) {
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, program->bottomTypes.intType));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->getBinaryExpr()->right = bitAndExpr();
    expr = std::move(binary);
  }
  return expr;
}
// Bit and expression
std::unique_ptr<Expr> Parser::bitAndExpr() {
  std::unique_ptr<Expr> expr = equateExpr();
  while (curr.type == TOKEN_TYPE::BITAND) {
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, program->bottomTypes.intType));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->getBinaryExpr()->right = equateExpr();
    expr = std::move(binary);
  }
  return expr;
}
// Equate expression (==, !=)
std::unique_ptr<Expr> Parser::equateExpr() {
  std::unique_ptr<Expr> expr = notExpr();
  while (curr.type == TOKEN_TYPE::EQUALS || curr.type == TOKEN_TYPE::NEQUALS) {
    std::unique_ptr<Expr> binary = std::make_unique<Expr>(
        Expr::makeBinary(curr, program->bottomTypes.intType));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->getBinaryExpr()->right = notExpr();
    expr = std::move(binary);
  }
  return expr;
}
// Not expression (!)
std::unique_ptr<Expr> Parser::notExpr() {
  if (curr.type == TOKEN_TYPE::NOT) {
    std::unique_ptr<Expr> expr = std::make_unique<Expr>(
        Expr{curr.sourceLocation, nullptr, PrefixExpr{curr}});
    curr = lexer.next();
    expr->getPrefixExpr()->expr = access();
    expr->type = expr->getPrefixExpr()->expr->type;
    return expr;
  } else {
    return relation();
  }
}
std::unique_ptr<Expr> Parser::relation() {
  std::unique_ptr<Expr> expr = shift();
  while (curr.type == TOKEN_TYPE::LANGLE || curr.type == TOKEN_TYPE::LEQ || curr.type == TOKEN_TYPE::RANGLE || curr.type == TOKEN_TYPE::GEQ) {
    std::unique_ptr<Expr> binary =
        std::make_unique<Expr>(Expr::makeBinary(curr, nullptr));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->type = binary->getBinaryExpr()->left->type;
    binary->getBinaryExpr()->right = shift();
    expr = std::move(binary);
  }
  return expr;
}
std::unique_ptr<Expr> Parser::shift() {
  std::unique_ptr<Expr> expr = add();
  while (curr.type == TOKEN_TYPE::LSHIFT || curr.type == TOKEN_TYPE::RSHIFT) {
    std::unique_ptr<Expr> binary =
        std::make_unique<Expr>(Expr::makeBinary(curr, nullptr));
    curr = lexer.next();
    binary->getBinaryExpr()->left = std::move(expr);
    binary->type = binary->getBinaryExpr()->left->type;
    binary->getBinaryExpr()->right = add();
    expr = std::move(binary);
  }
  return expr;
}
// Add (+ and -)
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
// Multiply (* and /)
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
// Negate (-)
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
// Access
std::unique_ptr<Expr> Parser::access() {
  std::unique_ptr<Expr> exp = primary();
  // Deal with convert (within parentheses, separated by comma)
  if (exp->isLiteralExpr() && exp->getLiteralExpr()->name == "convert") {
    auto typeHolder = std::make_unique<Expr>(
        Expr{curr.sourceLocation, nullptr, LiteralExpr{curr.text}});
    curr = lexer.next();
    while (!munch(TOKEN_TYPE::COMMA)) {
      if (curr.type == TOKEN_TYPE::FILE_END) return nullptr;
      typeHolder->getLiteralExpr()->name.append(curr.text);
      curr = lexer.next();
    }
    auto converter =
        std::make_unique<Expr>(Expr{exp->sourceLocation, nullptr, CallExpr{}});
    converter->getCallExpr()->expr = std::move(exp);
    converter->getCallExpr()->params.emplace_back(std::move(typeHolder));
    converter->getCallExpr()->params.emplace_back(std::move(expr()));
    eatCurr(TOKEN_TYPE::RIGHT_PAREN);
    return converter;
  }
  for (;;) {
    // Between parens
    if (curr.type == TOKEN_TYPE::LEFT_PAREN) {
      std::unique_ptr<Expr> func =
          std::make_unique<Expr>(curr.sourceLocation, nullptr, CallExpr{});
      func->getCallExpr()->expr = std::move(exp);
      curr = lexer.next();
      if (curr.type != TOKEN_TYPE::RIGHT_PAREN) {
        do {
          // Add parameters, separated by commas
          func->getCallExpr()->params.emplace_back(expr());
        } while (eatCurr(TOKEN_TYPE::COMMA));
      }
      exp = std::move(func);
      curr = lexer.next();
      return exp;
    } else if (curr.type == TOKEN_TYPE::DOT) {
      //.Identifier
      requireNext(TOKEN_TYPE::IDEN);
      std::unique_ptr<Expr> getter =
          std::make_unique<Expr>(curr.sourceLocation, nullptr, GetExpr{});
      getter->getGetExpr()->expr = std::move(exp);
      getter->getGetExpr()->name.name = curr.text;
      exp = std::move(getter);
      curr = lexer.next();
    } else {
      break;
    }
  }
  return exp;
}
// Fixer, view string in different form
std::string fixer(const std::string_view orig) {
  // Remove quotes
  const std::string_view no_quotes = orig.substr(1, orig.size() - 2);
  std::string returner;
  for (int i = 0; i < no_quotes.size(); ++i) {
    // Add non-escape sequences
    if (no_quotes[i] != '\\') {
      returner.push_back(no_quotes[i]);
      continue;
    }
    // Escape sequences
    if (no_quotes[i + 1] == 'x') {
      // Hexademical
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
// Deal with primary tokens, return the proper expression
std::unique_ptr<Expr> Parser::primary() {
  switch (curr.type) {
    // Int
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
          Expr::makeInt(curr, program->bottomTypes.intType, val));
      curr = lexer.next();
      return expr;
    }
    // Float
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
          Expr::makeFloat(curr, program->bottomTypes.floatType, val));
      curr = lexer.next();
      return expr;
    }
    // Left paren
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
    // Char
    case TOKEN_TYPE::CHAR: {
      // Regular character
      if (curr.text.size() == 3) {
        char c = curr.text.at(1);
        std::unique_ptr<Expr> returner = std::make_unique<Expr>(
            curr.sourceLocation, program->bottomTypes.charType, CharExpr(c));
        curr = lexer.next();
        return returner;
      } else if (curr.text.size() == 4) {
        // Escape sequences
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
            curr.sourceLocation, program->bottomTypes.charType, CharExpr(c));
        curr = lexer.next();
        return returner;
      } else {
        // Hexadecimal
        char c = std::stoi(std::string{curr.text.substr(3, 2)}, 0, 16);
        std::unique_ptr<Expr> returner = std::make_unique<Expr>(
            curr.sourceLocation, program->bottomTypes.charType, CharExpr(c));
        curr = lexer.next();
        return returner;
      }
    }
    // String
    case TOKEN_TYPE::STRING: {
      std::string correct = fixer(curr.text);
      std::unique_ptr<Expr> returner = std::make_unique<Expr>(
          curr.sourceLocation,
          std::make_shared<Type>(
              ListType(correct.size(), program->bottomTypes.charType),
              std::vector<std::shared_ptr<Impl>>{}),
          StringExpr(std::string{correct}));
      curr = lexer.next();
      return returner;
    }
    // Identifier
    case TOKEN_TYPE::IDEN: {
      std::unique_ptr<Expr> returner = std::make_unique<Expr>(
          curr.sourceLocation, program->bottomTypes.voidType,
          LiteralExpr(curr.text));
      curr = lexer.next();
      return returner;
    }
    // true
    case TOKEN_TYPE::TRUE: {
      std::unique_ptr<Expr> exp = std::make_unique<Expr>(
          curr.sourceLocation, program->bottomTypes.boolType, BoolExpr{true});
      curr = lexer.next();
      return exp;
    }
    // false
    case TOKEN_TYPE::FALSE: {
      std::unique_ptr<Expr> exp = std::make_unique<Expr>(
          curr.sourceLocation, program->bottomTypes.boolType, BoolExpr{false});
      curr = lexer.next();
      return exp;
    }
    // self
    case TOKEN_TYPE::SELF: {
      if (isImplClass != state::NORMAL) {
        std::unique_ptr<Expr> returner = std::make_unique<Expr>(
            curr.sourceLocation, program->bottomTypes.selfType,
            LiteralExpr(curr.text));
        curr = lexer.next();
        return returner;
      } else {
        std::cerr << "SELF cannot exist outside of an Impl or Class\n.";
        return nullptr;
      }
    }
    case TOKEN_TYPE::VOID: {
      auto returner = std::make_unique<Expr>(curr.sourceLocation, program->bottomTypes.voidType, VoidExpr{});
      curr = lexer.next();
      return returner;
    }
    default:
      // Otherwise, token is invalid
      std::cerr << "Invalid token at: " << curr.sourceLocation.line << ":"
                << curr.sourceLocation.character << "!\n";
      return {};
  }
}
