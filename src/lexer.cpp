// Copyright (c) 2024 Compile Collective. All Rights Reserved.
#include "lexer.h"

#include "token.h"

// Skip whitespace - increase line or position if necessary
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
// Returns true if character is a-z, A-Z, or an underscore
bool isAlpha(const char t) {
  return t == '_' || (t >= 'a' && t <= 'z') || (t >= 'A' && t <= 'Z');
}
// Returns true if a character is a digit 0-9
bool isNumeric(const char t) { return t >= '0' && t <= '9'; }
// Returns true if a character is a valid escape sequence (following \)
bool isEscapeSequence(const char t) {
  return t == 'a' || t == 'b' || t == 'f' || t == 'n' || t == 'r' || t == 't' ||
         t == 'v' || t == '\'' || t == '"' || t == '?' || t == '\\';
}
// Returns true if a character is a valid hexadecimal digit
bool isHex(const char t) {
  return isNumeric(t) || (t >= 'a' && t <= 'f') || (t >= 'A' && t <= 'F');
}
// Tokenize the next character
Token Lexer::next() {
  // Whitespace is ignored
  skipWhitespace();
  // End of file reached; final token
  if (pos >= input.size()) return Token{TOKEN_TYPE::FILE_END, "", curr};
  // Store length of current token
  size_t len = 1;
  // Assume error by default
  TOKEN_TYPE type = TOKEN_TYPE::ERROR;
  // Decide token based on characters in input
  switch (input.at(pos)) {
    case '+':
      type = TOKEN_TYPE::PLUS;
      break;
    case '-':
      type = TOKEN_TYPE::MINUS;
      // -> is arrow
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
      // != is not equals
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
      // Entire number is one token
      while (pos + len < input.size() && isNumeric(input.at(pos + len))) {
        ++len;
      }
      // Assume integer - if next token is . followed by another number, then it is a float
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
      // <= is less than or equal to, << is left shift
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
        // >= is greater than or equal to, >> is right shift
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
      // && is logical AND
      if (pos + len < input.size() && input.at(pos + len) == '&') {
        ++len;
        type = TOKEN_TYPE::AND;
      }
      break;
    case '=':
      type = TOKEN_TYPE::ASSIGN;
      // == is the equality operator
      if (pos + len < input.size() && input.at(pos + len) == '=') {
        ++len;
        type = TOKEN_TYPE::EQUALS;
      }
      break;
    case '|':
      type = TOKEN_TYPE::BITOR;
      // || is logical OR
      if (pos + len < input.size() && input.at(pos + len) == '|') {
        ++len;
        type = TOKEN_TYPE::OR;
      }
      break;
    case '.':
      type = TOKEN_TYPE::DOT;
      // .. is range
      if (pos + len < input.size() && input.at(pos + len) == '.') {
        ++len;
        type = TOKEN_TYPE::RANGE;
        // ..= is inclusive range
        if (pos + len < input.size() && input.at(pos + len) == '=') {
          ++len;
          type = TOKEN_TYPE::INCRANGE;
        }
      }
      break;
    case '"':
      ++len;
      // String includes all text between "'s
      while (pos + len < input.size()) {
        ++len;
        if (input.at(pos + len - 1) == '"') {
          type = TOKEN_TYPE::STRING;
          break;
        }
      }
      break;
    case '\'':
      // Character is between ''s (empty character)
      if (pos + len < input.size() && input.at(pos + len) == '\'') {
        ++len;
        type = TOKEN_TYPE::CHAR;
        break;
      }
      // Regular character
      if (pos + len + 1 < input.size() && input.at(pos + len) != '\\' &&
          input.at(pos + len + 1) == '\'') {
        len += 2;
        type = TOKEN_TYPE::CHAR;
        break;
      }
      // Escape sequence character
      if (pos + len + 2 < input.size() && input.at(pos + len) == '\\' &&
          isEscapeSequence(input.at(pos + len + 1)) &&
          input.at(pos + len + 2) == '\'') {
        len += 3;
        type = TOKEN_TYPE::CHAR;
        break;
      }
      // '\xHH' where H is a hex digit is a character (defines ASCII)
      if (pos + len + 4 < input.size() && input.at(pos + len) == '\\' &&
          input.at(pos + len + 1) == 'x' && isHex(input.at(pos + len + 2)) &&
          isHex(input.at(pos + len + 3)) && input.at(pos + len + 4) == '\'') {
        len += 5;
        type = TOKEN_TYPE::CHAR;
        break;
      }
    default:
      // If none of the above, and not alphabetical (a-z, A-Z, or _), input is not a valid token
      if (!isAlpha(input.at(pos))) break;
      // If the first character is alphabetical (including _), this is an identifier - add any subsequent alphabetical or numeric characters to it
      while (pos + len < input.size() &&
             (isAlpha(input.at(pos + len)) || isNumeric(input.at(pos + len)))) {
        ++len;
      }
      type = TOKEN_TYPE::IDEN;
      // Check for built-in identifiers (if none apply, remain an IDEN, i.e., a variable name)
      switch (input.at(pos)) {
        // case, const, continue, class
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
        // else
        case 'e':
          if (len == 4 && input.substr(pos + 1, len - 1) == "lse") {
            type = TOKEN_TYPE::ELSE;
          }
          break;
        // fn, false, for
        case 'f':
          if (len == 2 && input.at(pos + 1) == 'n') {
            type = TOKEN_TYPE::FN;
          } else if (len == 5 && input.substr(pos + 1, len - 1) == "alse") {
            type = TOKEN_TYPE::FALSE;
          } else if (len == 3 && input.substr(pos + 1, len - 1) == "or") {
            type = TOKEN_TYPE::FOR;
          }
          break;
        // if, in, impl
        case 'i':
          if (len == 2 && input.at(pos + 1) == 'f') {
            type = TOKEN_TYPE::IF;
          } else if (len == 2 && input.at(pos + 1) == 'n') {
            type = TOKEN_TYPE::IN;
          } else if (len == 4 && input.substr(pos + 1, len - 1) == "mpl") {
            type = TOKEN_TYPE::IMPL;
          }
          break;
        // let, list
        case 'l':
          if (len == 3 && input.substr(pos + 1, len - 1) == "et") {
            type = TOKEN_TYPE::LET;
          }
          if (len == 4 && input.substr(pos + 1, len - 1) == "ist") {
            type = TOKEN_TYPE::LIST;
          }
          break;
        // match
        case 'm':
          if (len == 5 && input.substr(pos + 1, len - 1) == "atch") {
            type = TOKEN_TYPE::MATCH;
          }
          break;
        // optional
        case 'o':
          if (len == 7 && input.substr(pos + 1, len - 1) == "ptional") {
            type = TOKEN_TYPE::OPTIONAL;
          }
          break;
        // return
        case 'r':
          if (len == 6 && input.substr(pos + 1, len - 1) == "eturn") {
            type = TOKEN_TYPE::RETURN;
          }
          break;
        // self
        case 's':
          if (len == 4 && input.substr(pos + 1, len - 1) == "elf") {
            type = TOKEN_TYPE::SELF;
          }
          break;
        // true, type
        case 't':
          if (len == 4 && input.substr(pos + 1, len - 1) == "rue") {
            type = TOKEN_TYPE::TRUE;
          } else if (len == 4 && input.substr(pos + 1, len - 1) == "ype") {
            type = TOKEN_TYPE::TYPE;
          }
          break;
        // void
        case 'v':
          if (len == 4 && input.substr(pos + 1, len - 1) == "oid") {
            type = TOKEN_TYPE::VOID;
          }
          break;
        // while
        case 'w':
          if (len == 5 && input.substr(pos + 1, len - 1) == "hile") {
            type = TOKEN_TYPE::WHILE;
          }
          break;
        // yield
        case 'y':
          if (len == 5 && input.substr(pos + 1, len - 1) == "ield") {
            type = TOKEN_TYPE::YIELD;
          }
          break;
      }
  }
  // If no valid token has been set, return an error
  if (type == TOKEN_TYPE::ERROR) return Token{type, "Invalid Character.", curr};
  // Create the valid token, update position, and return the token
  auto returner = Token{type, input.substr(pos, len), curr};
  pos += len;
  curr.character += len;
  return returner;
}
