#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include "at_interp.h"
#include "at_var.h"

extern void putch(char c);
extern int getln(char line[GETLN_LEN]);

static void putstrn(char const *s, size_t n) {
  while (n--)
    putch(*s++);
}

static void putstr(char const *s) {
  while (*s)
    putch(*s++);
}

enum token_type get_token_type(char c, enum token_type *previous) {
  if (previous && *previous == T_LABEL)
    if ((c >= 'A' && c <= 'Z') || c == '%' || c == '$')
      return T_LABEL;

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
  if (c == ',')
    return T_COMMA;
  return T_NONE;
}

size_t tokenize(char const **input, char const **out, enum token_type *token_type_out) {
  while (**input == ' ')
    ++*input;

  if (!**input) {
    *token_type_out = T_NONE;
    return 0;
  }

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


  if (*token_type_out == T_LABEL) {
    // TODO: clean up this mess
    if (*input - *out == 3 && strncmp(*out, "LET", 3) == 0) {
      *token_type_out = T_S_LET;
    } else if (*input - *out == 5 && strncmp(*out, "PRINT", 5) == 0) {
      *token_type_out = T_S_PRINT;
    } else if (*input - *out == 5 && strncmp(*out, "INPUT", 5) == 0) {
      *token_type_out = T_S_INPUT;
    }
  }

  return *input - *out;
}

static char const *t, *out, *accept_out;
static enum token_type token_type, accept_token_type;
static size_t token, accept_token;

static char const *tts(enum token_type tt) {
  switch (tt) {
  case T_NONE: return "T_NONE";
  case T_S_LET: return "T_S_LET";
  case T_S_PRINT: return "T_S_PRINT";
  case T_S_INPUT: return "T_S_INPUT";
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
  case T_COMMA: return "T_COMMA";
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

  static char EXPECT_ERR[40];
  snprintf(EXPECT_ERR, sizeof(EXPECT_ERR), "unexpected %s, expected %s", tts(token_type), tts(tt));
  *err = EXPECT_ERR;

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

  if (accept(T_S_LET)) {
    if (!expect(T_LABEL, err))
      return;

    char label[3] = {0, 0, 0};
    label[0] = accept_out[0];
    if (accept_token > 2)
      label[1] = accept_out[1];
    label[2] = accept_out[accept_token - 1];

    if (label[2] != '%' && label[2] != '$') {
      *err = "expected var name to end in % or $";
      return;
    }

    if (!expect(T_EQUAL, err))
      return;

    struct value v = exec_expr(err);
    if (*err)
      return;

    add_var(label, v, err);
    return;
  }

  if (accept(T_S_PRINT)) {
    struct value v = exec_expr(err);
    if (*err)
      return;

    switch (v.type) {
    case V_NUMBER: {
      char buf[10];
      snprintf(buf, sizeof(buf), "%d", v.as.number);
      putstr(buf);
      putstr("\n");
      break;
    }
    case V_STRING: {
      putstr(v.as.string);
      putstr("\n");
    }
    }
    return;
  }

  if (accept(T_S_INPUT)) {
    if (accept(T_STRING)) {
      putstrn(accept_out + 1, accept_token - 2);
      if (!expect(T_COMMA, err))
        return;
    }

    if (!expect(T_LABEL, err))
      return;

    char label[3] = {0, 0, 0};
    label[0] = accept_out[0];
    if (accept_token > 2)
      label[1] = accept_out[1];
    label[2] = accept_out[accept_token - 1];

    char line[GETLN_LEN];
    int len = getln(line);
    return;
  }

  *err = "invalid statement";
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

    if (v.type != v2.type) {
      *err = "type error";
      return v;
    }

    switch (binop) {
    case T_ADD:
      switch (v.type) {
      case V_NUMBER:
        v.as.number += v2.as.number;
        break;
      case V_STRING: {
        int l1 = strlen(v.as.string);
        int l2 = strlen(v2.as.string);
        if (l1 + l2 > MAX_STRING) {
          *err = "string too long";
          return v;
        }
        strcat(v.as.string, v2.as.string);
        break;
      }
      }
      break;

    case T_SUBTRACT:
      switch (v.type) {
      case V_NUMBER:
        v.as.number -= v2.as.number;
        break;
      case V_STRING:
        *err = "type error";
        return v;
      }
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

    if (v.type != v2.type) {
      *err = "type error";
      return v;
    }

    switch (binop) {
    case T_MULTIPLY:
      switch (v.type) {
      case V_NUMBER:
        v.as.number *= v2.as.number;
        break;
      case V_STRING:
        *err = "type error";
        return v;
      }
      break;
    case T_DIVIDE:
      switch (v.type) {
      case V_NUMBER:
        v.as.number /= v2.as.number;
        break;
      case V_STRING:
        *err = "type error";
        return v;
      }
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

  if (accept(T_LABEL)) {
    char label[3] = {0, 0, 0};
    label[0] = accept_out[0];
    if (accept_token > 2)
      label[1] = accept_out[1];
    label[2] = accept_out[accept_token - 1];

    v = get_var(label, err);
    if (*err) {
      return v;
    }

    return v;
  }

  if (accept(T_STRING)) {
    v.type = V_STRING;
    int len = accept_token - 2;
    if (len > MAX_STRING)
      len = MAX_STRING;
    memcpy(v.as.string, accept_out + 1, len);
    v.as.string[len] = 0;
    return v;
  }

  *err = "expected factor";
  return v;
}
