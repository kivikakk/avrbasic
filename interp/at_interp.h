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
size_t tokenize(char const **input, char const **out, enum token_type *token_type_out);

#endif
