#ifndef AT_INTERP_H
#define AT_INTERP_H

#include <stdint.h>

enum token_type {
  T_NONE,
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
};

struct value {
  enum value_type type;
  union {
    int16_t number;
  } as;
};

enum token_type get_token_type(char c, enum token_type *previous);
size_t tokenize(char const **input, char const **out, enum token_type *token_type_out);
void prep(char const *text);
struct value exec_expr(char const **err);

#endif
