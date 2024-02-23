// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#include "lexer.h"

void Lexer::skipWhitespace() {
  while (pos < input.size() &&
         (input.at(pos) == ' ' || input.at(pos) == '\t' ||
          input.at(pos) == '\r' || input.at(pos) == '\n')) {
    ++curr.character;
    if (input.at(pos) == '\n') {
      ++curr.line;
      curr.character = 1;
    }
    ++pos;
  }
}
bool isNumeric(const char t) {
  return t >= '0' && t <= '9';
}
Token Lexer::next() {
  skipWhitespace();
  if (pos >= input.size()) return Token{TOKEN_TYPE::FILE_END, "", curr};
  size_t len = 1;
  TOKEN_TYPE type = TOKEN_TYPE::ERROR;
  switch (input.at(pos)) {
    case '+':
      type = TOKEN_TYPE::PLUS;
      break;
    case '-':
      type = TOKEN_TYPE::MINUS;
      break;
    case '*':
      type = TOKEN_TYPE::STAR;
      break;
    case '/':
      type = TOKEN_TYPE::SLASH;
      break;
    case '(':
      type = TOKEN_TYPE::LEFT_PAREN;
      break;
    case ')':
      type = TOKEN_TYPE::RIGHT_PAREN;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      while (pos + len < input.size() && isNumeric(input.at(pos+len))) {
        ++len;
      }
      type = TOKEN_TYPE::INT;
      if(input.at(pos + len) == '.') {
        type = TOKEN_TYPE::FLOAT;
        ++len;
      }
      while(pos + len < input.size() && isNumeric(input.at(pos+len))) {
        ++len;
      }
      break;
  }
  if (type == TOKEN_TYPE::ERROR) return Token{type, "Invalid Character.", curr};
  auto returner = Token{type, input.substr(pos, len), curr};
  pos += len;
  curr.character += len;
  return returner;
}
