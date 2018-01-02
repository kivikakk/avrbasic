#ifndef AT_INTERP_H
#define AT_INTERP_H

enum token_type {
  NONE,
  NUMBER,
  LABEL,
  BINOP,
  STRING,
};

enum token_type get_token_type(char c, enum token_type *previous);

#endif
