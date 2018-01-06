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

static bool expect(enum token_type tt, char const **err) {
  if (accept(tt))
    return true;
  *err = "unexpected symbol";
#ifdef TEST
  fprintf(stderr, "unexpected symbol %s, expected %s\n", tts(token_type), tts(tt));
#endif
  return false;
}

void prep(char const *text) {
  t = text;
  nextsym();
}

void exec_stmt(char const *stmt, char const **err) {
  t = stmt;

  if (!nextsym())
    return;

  if (token == 3 && strncmp(out, "LET", 3) == 0) {
    if (!nextsym()) {
      *err = "expected token after LET";
      return;
    }

    if (token_type != T_LABEL) {
      *err = "expected label after LET";
      return;
    }

    if (!nextsym()) {
      *err = "expected token after LET label";
      return;
    }

    if (!expect(T_EQUAL, err)) {
      return;
    }
  }
}

enum binop {
  ADD,
  SUBTRACT,
  MULTIPLY,
  DIVIDE,
  EQUAL,
};

static struct value outer(char const **err);
static struct value term(char const **err);
static struct value factor(char const **err);

struct value exec_expr(char const **err) {
  struct value v = outer(err);
  if (*err)
    return v;

  while (accept(T_EQUAL)) {
    struct value v2 = outer(err);
    if (*err)
      return v;

    v.as.number = (v.as.number == v2.as.number);
  }

  return v;
}

static struct value outer(char const **err) {
  struct value v = term(err);
  if (*err)
    return v;

  while (accept(T_ADD) || accept(T_SUBTRACT)) {
    enum token_type binop = accept_token_type;

    struct value v2 = term(err);
    if (*err)
      return v;

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

static struct value term(char const **err) {
  struct value v = factor(err);
  if (*err)
    return v;

  while (accept(T_MULTIPLY) || accept(T_DIVIDE)) {
    enum token_type binop = accept_token_type;

    struct value v2 = factor(err);
    if (*err)
      return v;

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

static struct value factor(char const **err) {
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
    v = exec_expr(err);
    expect(T_RPAREN, err);
    return v;
  }

  *err = "expected factor";
  return v;
}
