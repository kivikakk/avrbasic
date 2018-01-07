#ifndef AT_INTERP_H
#define AT_INTERP_H

#include <stdlib.h>
#include <stdint.h>

#define MAX_STRING 25
#define GETLN_LEN 80
#define ERR_LEN 80

enum token_type {
  T_NONE,
  T_S_LET,
  T_S_PRINT,
  T_S_INPUT,
  T_S_IF,
  T_S_THEN,
  T_S_ELSE,
  T_S_ELSEIF,
  T_S_END,
  T_S_LIST,
  T_S_RUN,
  T_NUMBER,
  T_LABEL,
  T_ADD,
  T_SUBTRACT,
  T_MULTIPLY,
  T_DIVIDE,
  T_EQUAL,
  T_STRING,
  T_LPAREN,
  T_RPAREN,
  T_COMMA,
};

enum value_type {
  V_NUMBER,
  V_STRING,
};

struct value {
  enum value_type type;
  union {
    int16_t number;
    char string[MAX_STRING + 1];
  } as;
};

enum token_type get_token_type(char c, enum token_type *previous);
size_t tokenize(char const **input, char const **out, enum token_type *token_type_out);
void prep(char const *text);
struct value exec_expr(char *err);
void exec_stmt(char const *stmt, char *err);

#endif
