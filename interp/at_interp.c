#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#ifdef TEST
#  include <stdio.h>
#endif
#include "at_interp.h"

enum token_type get_token_type(char c, enum token_type *previous) {
  if (previous && *previous == T_LABEL) {
    if ((c >= 'A' && c <= 'Z') || c == '%' || c == '$')
      return T_LABEL;
  }

  if (previous && *previous == T_STRING)
    return T_STRING;

  if (c >= '0' && c <= '9')
    return T_NUMBER;
  if (c >= 'A' && c <= 'Z')
    return T_LABEL;
  if (c == '+')
    return T_ADD;
  if (c == '-')
    return T_SUBTRACT;
  if (c == '*')
    return T_MULTIPLY;
  if (c == '/')
    return T_DIVIDE;
  if (c == '=')
    return T_EQUAL;
  if (c == '"')
    return T_STRING;
  if (c == '(')
    return T_LPAREN;
  if (c == ')')
    return T_RPAREN;
  return T_NONE;
}

size_t tokenize(char const **input, char const **out, enum token_type *token_type_out) {
  while (**input == ' ') {
    ++*input;
  }

  if (!**input)
    return 0;

  *out = *input;
  *token_type_out = get_token_type(**input, NULL);
  if (*token_type_out == T_NONE)
    return 0;

  while (**input && get_token_type(**input, token_type_out) == *token_type_out) {
    if (*input != *out && **input == '"' && *token_type_out == T_STRING) {
      ++*input;
      break;
    }
    ++*input;
  }

  return *input - *out;
}

static char const *t, *out, *accept_out;
static enum token_type token_type, accept_token_type;
static size_t token, accept_token;

static char const *tts(enum token_type tt) {
  switch (tt) {
  case T_NONE: return "T_NONE";
  case T_NUMBER: return "T_NUMBER";
  case T_LABEL: return "T_LABEL";
  case T_ADD: return "T_ADD";
  case T_SUBTRACT: return "T_SUBTRACT";
  case T_MULTIPLY: return "T_MULTIPLY";
  case T_DIVIDE: return "T_DIVIDE";
  case T_EQUAL: return "T_EQUAL";
  case T_STRING: return "T_STRING";
  case T_LPAREN: return "T_LPAREN";
  case T_RPAREN: return "T_RPAREN";
  default: return "unknown";
  }
}

static bool nextsym(void) {
  token = tokenize(&t, &out, &token_type);
  return token > 0;
}

static bool accept(enum token_type tt) {
  if (token_type == tt) {
    accept_out = out;
    accept_token = token;
    accept_token_type = token_type;
    nextsym();
    return true;
  }
  return false;
}

static bool expect(enum token_type tt) {
  if (accept(tt))
    return true;
#ifdef TEST
  fprintf(stderr, "unexpected symbol %s, expected %s\n", tts(token_type), tts(tt));
#endif
  return false;
}

void prep(char const *text) {
  t = text;
  nextsym();
}

char const *exec_stmt(char const *stmt) {
  t = stmt;

  if (!nextsym())
    return NULL;

  if (token == 3 && strncmp(out, "LET", 3) == 0) {
    if (!nextsym())
      return "expected token after LET";

    if (token_type != T_LABEL)
      return "expected label after LET";

    if (!nextsym())
      return "expected token after LET label";

    if (!expect(T_EQUAL))
      return "expected = after LET label";
  }

  return NULL;
}

enum binop {
  ADD,
  SUBTRACT,
  MULTIPLY,
  DIVIDE,
  EQUAL,
};

static struct value term(void);
static struct value factor(void);

struct value exec_expr(void) {
  struct value v = term();

  while (accept(T_ADD) || accept(T_SUBTRACT)) {
    enum token_type binop = accept_token_type;

    struct value v2 = factor();

    switch (binop) {
    case T_ADD:
      v.as.number += v2.as.number;
      break;
    case T_SUBTRACT:
      v.as.number -= v2.as.number;
      break;
    default:
      assert(false);
    }
  }

  return v;
}

static struct value term(void) {
  struct value v = factor();
  while (accept(T_MULTIPLY) || accept(T_DIVIDE)) {
    enum token_type binop = accept_token_type;
    
    struct value v2 = factor();

    switch (binop) {
    case T_MULTIPLY:
      v.as.number *= v2.as.number;
      break;
    case T_DIVIDE:
      v.as.number /= v2.as.number;
      break;
    default:
      assert(false);
    }
  }

  return v;
}

static struct value factor(void) {
  struct value v;
  v.type = V_NUMBER;
  v.as.number = -32768;

  if (accept(T_NUMBER)) {
    char num[40];
    if (accept_token >= sizeof(num)) {
      accept_token = sizeof(num) - 1;
    }
    strncpy(num, accept_out, accept_token);
    num[accept_token] = 0;
    v.as.number = atoi(num);
    return v;
  }

  if (accept(T_LPAREN)) {
    v = exec_expr();
    expect(T_RPAREN);
    return v;
  }

  return v;
}
