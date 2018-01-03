#ifndef AT_INTERP_H
#define AT_INTERP_H

enum token_type {
  T_NONE,
  T_NUMBER,
  T_LABEL,
  T_BINOP,
  T_STRING,
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
struct value exec_expr(char const *expr);

#endif
