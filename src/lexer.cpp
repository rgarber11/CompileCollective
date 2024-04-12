// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#include "lexer.h"

#include "token.h"

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
bool isAlpha(const char t) {
  return t == '_' || (t >= 'a' && t <= 'z') || (t >= 'A' && t <= 'Z');
}
bool isNumeric(const char t) { return t >= '0' && t <= '9'; }
bool isEscapeSequence(const char t) {
  return t == 'a' || t == 'b' || t == 'f' || t == 'n' || t == 'r' || t == 't' ||
         t == 'v' || t == '\'' || t == '"' || t == '?' || t == '\\';
}
bool isHex(const char t) {
  return isNumeric(t) || (t >= 'a' && t <= 'f') || (t >= 'A' && t <= 'F');
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
      if (pos + len < input.size() && input.at(pos + len) == '>') {
        ++len;
        type = TOKEN_TYPE::ARROW;
      }
      break;
    case '*':
      type = TOKEN_TYPE::STAR;
      break;
    case '/':
      type = TOKEN_TYPE::SLASH;
      break;
    case '%':
      type = TOKEN_TYPE::MOD;
      break;
    case '(':
      type = TOKEN_TYPE::LEFT_PAREN;
      break;
    case ')':
      type = TOKEN_TYPE::RIGHT_PAREN;
      break;
    case ';':
      type = TOKEN_TYPE::SEMI;
      break;
    case ':':
      type = TOKEN_TYPE::COLON;
      break;
    case '!':
      type = TOKEN_TYPE::NOT;
      if (pos + len < input.size() && input.at(pos + len) == '=') {
        ++len;
        type = TOKEN_TYPE::NEQUALS;
      }
      break;
    case '^':
      type = TOKEN_TYPE::XOR;
      break;
    case '[':
      type = TOKEN_TYPE::LSQUARE;
      break;
    case ']':
      type = TOKEN_TYPE::RSQUARE;
      break;
    case '{':
      type = TOKEN_TYPE::LBRACKET;
      break;
    case '}':
      type = TOKEN_TYPE::RBRACKET;
      break;
    case ',':
      type = TOKEN_TYPE::COMMA;
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
      while (pos + len < input.size() && isNumeric(input.at(pos + len))) {
        ++len;
      }
      type = TOKEN_TYPE::INT;
      if (pos + len + 1 < input.size() && input.at(pos + len) == '.' &&
          isNumeric(input.at(pos + len + 1))) {
        type = TOKEN_TYPE::FLOAT;
        len += 2;
      }
      while (pos + len < input.size() && isNumeric(input.at(pos + len))) {
        ++len;
      }
      break;
    case '<':
      type = TOKEN_TYPE::LANGLE;
      if (pos + len >= input.size()) break;
      switch (input.at(pos + len)) {
        case '=':
          ++len;
          type = TOKEN_TYPE::LEQ;
          break;
        case '<':
          ++len;
          type = TOKEN_TYPE::LSHIFT;
          break;
      }
      break;
    case '>':
      type = TOKEN_TYPE::RANGLE;
      if (pos + len >= input.size()) break;
      switch (input.at(pos + len)) {
        case '=':
          ++len;
          type = TOKEN_TYPE::GEQ;
          break;
        case '>':
          ++len;
          type = TOKEN_TYPE::RSHIFT;
          break;
      }
      break;
    case '&':
      type = TOKEN_TYPE::BITAND;
      if (pos + len < input.size() && input.at(pos + len) == '&') {
        ++len;
        type = TOKEN_TYPE::AND;
      }
      break;
    case '=':
      type = TOKEN_TYPE::ASSIGN;
      if (pos + len < input.size() && input.at(pos + len) == '=') {
        ++len;
        type = TOKEN_TYPE::EQUALS;
      }
      break;
    case '|':
      type = TOKEN_TYPE::BITOR;
      if (pos + len < input.size() && input.at(pos + len) == '|') {
        ++len;
        type = TOKEN_TYPE::OR;
      }
      break;
    case '.':
      type = TOKEN_TYPE::DOT;
      if (pos + len < input.size() && input.at(pos + len) == '.') {
        ++len;
        type = TOKEN_TYPE::RANGE;
        if (pos + len < input.size() && input.at(pos + len) == '=') {
          ++len;
          type = TOKEN_TYPE::INCRANGE;
        }
      }
      break;
    case '"':
      ++len;
      while (pos + len < input.size()) {
        ++len;
        if (input.at(pos + len - 1) == '"') {
          type = TOKEN_TYPE::STRING;
          break;
        }
      }
      break;
    case '\'':
      if (pos + len < input.size() && input.at(pos + len) == '\'') {
        ++len;
        type = TOKEN_TYPE::CHAR;
        break;
      }
      if (pos + len + 1 < input.size() && input.at(pos + len) != '\\' &&
          input.at(pos + len + 1) == '\'') {
        len += 2;
        type = TOKEN_TYPE::CHAR;
        break;
      }
      if (pos + len + 2 < input.size() && input.at(pos + len) == '\\' &&
          isEscapeSequence(input.at(pos + len + 1)) &&
          input.at(pos + len + 2) == '\'') {
        len += 3;
        type = TOKEN_TYPE::CHAR;
        break;
      }
      if (pos + len + 4 < input.size() && input.at(pos + len) == '\\' &&
          input.at(pos + len + 1) == 'x' && isHex(input.at(pos + len + 2)) &&
          isHex(input.at(pos + len + 3)) && input.at(pos + len + 4) == '\'') {
        len += 5;
        type = TOKEN_TYPE::CHAR;
        break;
      }
    default:
      if (!isAlpha(input.at(pos))) break;
      while (pos + len < input.size() &&
             (isAlpha(input.at(pos + len)) || isNumeric(input.at(pos + len)))) {
        ++len;
      }
      type = TOKEN_TYPE::IDEN;
      switch (input.at(pos)) {
        case 'c':
          if (len == 4 && input.substr(pos + 1, len - 1) == "ase") {
            type = TOKEN_TYPE::CASE;
          } else if (len == 5 && input.substr(pos + 1, len - 1) == "onst") {
            type = TOKEN_TYPE::CONST;
          } else if (len == 8 && input.substr(pos + 1, len - 1) == "ontinue") {
            type = TOKEN_TYPE::CONTINUE;
          } else if (len == 5 && input.substr(pos + 1, len - 1) == "lass") {
            type = TOKEN_TYPE::CLASS;
          }
          break;
        case 'e':
          if (len == 4 && input.substr(pos + 1, len - 1) == "lse") {
            type = TOKEN_TYPE::ELSE;
          }
          break;
        case 'f':
          if (len == 2 && input.at(pos + 1) == 'n') {
            type = TOKEN_TYPE::FN;
          } else if (len == 5 && input.substr(pos + 1, len - 1) == "alse") {
            type = TOKEN_TYPE::FALSE;
          } else if (len == 3 && input.substr(pos + 1, len - 1) == "or") {
            type = TOKEN_TYPE::FOR;
          }
          break;
        case 'i':
          if (len == 2 && input.at(pos + 1) == 'f') {
            type = TOKEN_TYPE::IF;
          } else if (len == 2 && input.at(pos + 1) == 'n') {
            type = TOKEN_TYPE::IN;
          } else if (len == 4 && input.substr(pos + 1, len - 1) == "mpl") {
            type = TOKEN_TYPE::IMPL;
          }
          break;
        case 'l':
          if (len == 3 && input.substr(pos + 1, len - 1) == "et") {
            type = TOKEN_TYPE::LET;
          }
          if (len == 4 && input.substr(pos + 1, len - 1) == "ist") {
            type = TOKEN_TYPE::LIST;
          }
          break;
        case 'm':
          if (len == 5 && input.substr(pos + 1, len - 1) == "atch") {
            type = TOKEN_TYPE::MATCH;
          }
          break;
        case 'o':
          if (len == 7 && input.substr(pos + 1, len - 1) == "ptional") {
            type = TOKEN_TYPE::OPTIONAL;
          }
          break;
        case 'r':
          if (len == 6 && input.substr(pos + 1, len - 1) == "eturn") {
            type = TOKEN_TYPE::RETURN;
          }
          break;
        case 's':
          if (len == 4 && input.substr(pos + 1, len - 1) == "elf") {
            type = TOKEN_TYPE::SELF;
          }
          break;
        case 't':
          if (len == 4 && input.substr(pos + 1, len - 1) == "rue") {
            type = TOKEN_TYPE::TRUE;
          } else if (len == 4 && input.substr(pos + 1, len - 1) == "ype") {
            type = TOKEN_TYPE::TYPE;
          }
          break;
        case 'v':
          if (len == 4 && input.substr(pos + 1, len - 1) == "oid") {
            type = TOKEN_TYPE::VOID;
          }
          break;
        case 'w':
          if (len == 5 && input.substr(pos + 1, len - 1) == "hile") {
            type = TOKEN_TYPE::WHILE;
          }
          break;
        case 'y':
          if (len == 5 && input.substr(pos + 1, len - 1) == "ield") {
            type = TOKEN_TYPE::YIELD;
          }
          break;
      }
  }
  if (type == TOKEN_TYPE::ERROR) return Token{type, "Invalid Character.", curr};
  auto returner = Token{type, input.substr(pos, len), curr};
  pos += len;
  curr.character += len;
  return returner;
}
