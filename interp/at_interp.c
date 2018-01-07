#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include "at_interp.h"
#include "at_var.h"
#include "at_pgrm.h"
#include "pgmspace.h"

extern void putch(char c);
extern int getln(char line[GETLN_LEN]);
extern void flush(void);

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
    } else if (*input - *out == 2 && strncmp(*out, "IF", 2) == 0) {
      *token_type_out = T_S_IF;
    } else if (*input - *out == 4 && strncmp(*out, "THEN", 4) == 0) {
      *token_type_out = T_S_THEN;
    } else if (*input - *out == 4 && strncmp(*out, "ELSE", 4) == 0) {
      *token_type_out = T_S_ELSE;
    } else if (*input - *out == 6 && strncmp(*out, "ELSEIF", 6) == 0) {
      *token_type_out = T_S_ELSEIF;
    } else if (*input - *out == 3 && strncmp(*out, "END", 3) == 0) {
      *token_type_out = T_S_END;
    } else if (*input - *out == 4 && strncmp(*out, "LIST", 4) == 0) {
      *token_type_out = T_S_LIST;
    }
  }

  return *input - *out;
}

static char const *t, *out, *accept_out;
static enum token_type token_type, accept_token_type;
static size_t token, accept_token;

char const N_T_NONE[] PROGMEM = "T_NONE";
char const N_T_S_LET[] PROGMEM = "T_S_LET";
char const N_T_S_PRINT[] PROGMEM = "T_S_PRINT";
char const N_T_S_INPUT[] PROGMEM = "T_S_INPUT";
char const N_T_S_IF[] PROGMEM = "T_S_IF";
char const N_T_S_THEN[] PROGMEM = "T_S_THEN";
char const N_T_S_ELSE[] PROGMEM = "T_S_ELSE";
char const N_T_S_ELSEIF[] PROGMEM = "T_S_ELSEIF";
char const N_T_S_END[] PROGMEM = "T_S_END";
char const N_T_S_LIST[] PROGMEM = "T_S_LIST";
char const N_T_NUMBER[] PROGMEM = "T_NUMBER";
char const N_T_LABEL[] PROGMEM = "T_LABEL";
char const N_T_ADD[] PROGMEM = "T_ADD";
char const N_T_SUBTRACT[] PROGMEM = "T_SUBTRACT";
char const N_T_MULTIPLY[] PROGMEM = "T_MULTIPLY";
char const N_T_DIVIDE[] PROGMEM = "T_DIVIDE";
char const N_T_EQUAL[] PROGMEM = "T_EQUAL";
char const N_T_STRING[] PROGMEM = "T_STRING";
char const N_T_LPAREN[] PROGMEM = "T_LPAREN";
char const N_T_RPAREN[] PROGMEM = "T_RPAREN";
char const N_T_COMMA[] PROGMEM = "T_COMMA";
char const N_unknown[] PROGMEM = "unknown";

static PGM_P tts(enum token_type tt) {
  switch (tt) {
  case T_NONE: return N_T_NONE;
  case T_S_LET: return N_T_S_LET;
  case T_S_PRINT: return N_T_S_PRINT;
  case T_S_INPUT: return N_T_S_INPUT;
  case T_S_IF: return N_T_S_IF;
  case T_S_THEN: return N_T_S_THEN;
  case T_S_ELSE: return N_T_S_ELSE;
  case T_S_ELSEIF: return N_T_S_ELSEIF;
  case T_S_END: return N_T_S_END;
  case T_S_LIST: return N_T_S_LIST;
  case T_NUMBER: return N_T_NUMBER;
  case T_LABEL: return N_T_LABEL;
  case T_ADD: return N_T_ADD;
  case T_SUBTRACT: return N_T_SUBTRACT;
  case T_MULTIPLY: return N_T_MULTIPLY;
  case T_DIVIDE: return N_T_DIVIDE;
  case T_EQUAL: return N_T_EQUAL;
  case T_STRING: return N_T_STRING;
  case T_LPAREN: return N_T_LPAREN;
  case T_RPAREN: return N_T_RPAREN;
  case T_COMMA: return N_T_COMMA;
  default: return N_unknown;
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

char const UNEXPECTED1_ERR[] PROGMEM = "unexpected ";
char const UNEXPECTED2_ERR[] PROGMEM = ", expected ";

static bool expect(enum token_type tt, char *err) {
  if (accept(tt))
    return true;

  strcpy_P(err, UNEXPECTED1_ERR);
  strcat_P(err, tts(token_type));
  strcat_P(err, UNEXPECTED2_ERR);
  strcat_P(err, tts(tt));
  return false;
}

void prep(char const *text) {
  t = text;
  nextsym();
}

const char INVALID_STMT_ERR[] PROGMEM = "invalid statement";

static void exec_stmt_let(char *err);
static void exec_stmt_print(char *err);
static void exec_stmt_input(char *err);
static void exec_stmt_lno(char *err);
static void exec_stmt_list(char *err);

void exec_stmt(char const *stmt, char *err) {
  t = stmt;

  if (!nextsym())
    return;

  if (accept(T_S_LET)) {
    exec_stmt_let(err);
    return;
  }

  if (accept(T_S_PRINT)) {
    exec_stmt_print(err);
    return;
  }

  if (accept(T_S_INPUT)) {
    exec_stmt_input(err);
    return;
  }

  if (accept(T_NUMBER)) {
    exec_stmt_lno(err);
    return;
  }

  if (accept(T_S_LIST)) {
    exec_stmt_list(err);
    return;
  }

  strcpy_P(err, INVALID_STMT_ERR);
}

const char VAR_SIGIL_ERR[] PROGMEM = "expected var name to end in % or $";

static void exec_stmt_let(char *err) {
  if (!expect(T_LABEL, err))
    return;

  char label[3] = {0, 0, 0};
  label[0] = accept_out[0];
  if (accept_token > 2)
    label[1] = accept_out[1];
  label[2] = accept_out[accept_token - 1];

  if (label[2] != '%' && label[2] != '$') {
    strcpy_P(err, VAR_SIGIL_ERR);
    return;
  }

  if (!expect(T_EQUAL, err))
    return;

  struct value v = exec_expr(err);
  if (*err)
    return;

  add_var(label, v, err);
  if (*err)
    return;

  expect(T_NONE, err);
}

static void exec_stmt_print(char *err) {
  do {
    struct value v = exec_expr(err);
    if (*err)
      return;

    switch (v.type) {
    case V_NUMBER: {
      char buf[10];
      snprintf(buf, sizeof(buf), "%d", v.as.number);
      putstr(buf);
      break;
    }
    case V_STRING: {
      putstr(v.as.string);
    }
    }
  } while (accept(T_COMMA));

  putstr("\n");
  flush();

  expect(T_NONE, err);
}

const char INPUT_UNKNOWN_VAR_ERR[] PROGMEM = "INPUT encountered unknown var";

static void exec_stmt_input(char *err) {
  if (accept(T_STRING)) {
    putstrn(accept_out + 1, accept_token - 2);
    flush();
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

  char line[GETLN_LEN + 1];
  int len = getln(line);
  line[len] = 0;

  struct value v;
  if (label[2] == '%') {
    v.type = V_NUMBER;
    v.as.number = atoi(line);
  } else if (label[2] == '$') {
    v.type = V_STRING;
    if (len > MAX_STRING)
      len = MAX_STRING;
    memcpy(v.as.string, line, len);
    v.as.string[len] = 0;
  } else {
    strcpy_P(err, INPUT_UNKNOWN_VAR_ERR);
    return;
  }

  add_var(label, v, err);
  if (*err)
    return;

  expect(T_NONE, err);
}

static void exec_stmt_lno(char *err) {
  uint16_t lno = atoi(accept_out);
  if (token_type != T_NONE) {
    add_line(lno, out, err);
    return;
  }

  char lineno[7];
  char line[MAX_LINE_LEN + 1];
  int len = get_line(lno, line, err);
  if (*err)
    return;
  line[len] = 0;
  snprintf(lineno, sizeof(lineno), "%d ", lno);
  putstr(lineno);
  putstr(line);
  putstr("\n");
}

static void exec_stmt_list(char *err) {
  char line[MAX_LINE_LEN + 1];
  char lineno[7];

  for (uint16_t lno = MIN_LINE; lno <= MAX_LINE; ++lno) {
    int len = get_line(lno, line, err);
    if (*err)
      return;
    if (!len)
      continue;
    line[len] = 0;
    snprintf(lineno, sizeof(lineno), "%d ", lno);
    putstr(lineno);
    putstr(line);
    putstr("\n");
  }
}

enum binop {
  ADD,
  SUBTRACT,
  MULTIPLY,
  DIVIDE,
  EQUAL,
};

static struct value outer(char *err);
static struct value term(char *err);
static struct value factor(char *err);

struct value exec_expr(char *err) {
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

const char TYPE_ERR[] PROGMEM = "type error";
const char STRING_LEN_ERR[] PROGMEM = "string too long";
const char UNTERMINATED_STRING_ERR[] PROGMEM = "unterminated string";
const char FACTOR_ERR[] PROGMEM = "expected factor";

static struct value outer(char *err) {
  struct value v = term(err);
  if (*err)
    return v;

  while (accept(T_ADD) || accept(T_SUBTRACT)) {
    enum token_type binop = accept_token_type;

    struct value v2 = term(err);
    if (*err)
      return v;

    if (v.type != v2.type) {
      strcpy_P(err, TYPE_ERR);
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
          strcpy_P(err, STRING_LEN_ERR);
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
        strcpy_P(err, TYPE_ERR);
        return v;
      }
      break;
    default:
      assert(false);
    }
  }

  return v;
}

static struct value term(char *err) {
  struct value v = factor(err);
  if (*err)
    return v;

  while (accept(T_MULTIPLY) || accept(T_DIVIDE)) {
    enum token_type binop = accept_token_type;

    struct value v2 = factor(err);
    if (*err)
      return v;

    if (v.type != v2.type) {
      strcpy_P(err, TYPE_ERR);
      return v;
    }

    switch (binop) {
    case T_MULTIPLY:
      switch (v.type) {
      case V_NUMBER:
        v.as.number *= v2.as.number;
        break;
      case V_STRING:
        strcpy_P(err, TYPE_ERR);
        return v;
      }
      break;
    case T_DIVIDE:
      switch (v.type) {
      case V_NUMBER:
        v.as.number /= v2.as.number;
        break;
      case V_STRING:
        strcpy_P(err, TYPE_ERR);
        return v;
      }
      break;
    default:
      assert(false);
    }
  }

  return v;
}

static struct value factor(char *err) {
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
    if (*err)
      return v;

    return v;
  }

  if (accept(T_STRING)) {
    v.type = V_STRING;
    int len = accept_token - 2;
    if (len > MAX_STRING)
      len = MAX_STRING;
    if (len < 0) {
      strcpy_P(err, UNTERMINATED_STRING_ERR);
      return v;
    }
    memcpy(v.as.string, accept_out + 1, len);
    v.as.string[len] = 0;
    return v;
  }

  strcpy_P(err, FACTOR_ERR);
  return v;
}
