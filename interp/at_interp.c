#include <stdlib.h>
#include <string.h>
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
  if (c == '+' || c == '-' || c == '=')
    return T_BINOP;
  if (c == '"')
    return T_STRING;
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

char const *exec_stmt(char const *stmt) {
  char const *t = stmt, *out;
  enum token_type token_type;
  size_t token;

  if ((token = tokenize(&t, &out, &token_type)) == 0)
    return NULL;

  if (token == 3 && strncmp(out, "LET", 3) == 0) {
    if ((token = tokenize(&t, &out, &token_type)) == 0) {
      return "expected token after LET";
    }

    if (token_type != T_LABEL) {
      return "expected label after LET";
    }

    if ((token = tokenize(&t, &out, &token_type)) == 0) {
      return "expected token after LET label";
    }

    if (token_type != T_BINOP || token != 1 || strncmp(out, "=", 1) != 0) {
      return "expected = after LET label";
    }

    exec_expr(t);
  }

  return NULL;
}

struct value exec_expr(char const *expr) {
  struct value v;
  char const *t = expr, *out;
  enum token_type token_type;
  size_t token;

  v.type = V_NUMBER;
  v.as.number = -1;

  if ((token = tokenize(&t, &out, &token_type)) == 0) {
    // TODO ERROR
    return v;
  }

  
  return v;
}
