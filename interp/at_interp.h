#ifndef AT_INTERP_H
#define AT_INTERP_H

#include <stdlib.h>
#include <stdint.h>

#define MAX_STRING 25

enum token_type {
  T_NONE,
  T_S_LET,
  T_S_PRINT,
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
struct value exec_expr(char const **err);
void exec_stmt(char const *stmt, char const **err);

#endif
