#include <stdlib.h>
#include "at_interp.h"

enum token_type get_token_type(char c, enum token_type *previous) {
  if (previous && *previous == LABEL) {
    if ((c >= 'A' && c <= 'Z') || c == '%' || c == '$')
      return LABEL;
  }

  if (previous && *previous == STRING)
    return STRING;

  if (c >= '0' && c <= '9')
    return NUMBER;
  if (c >= 'A' && c <= 'Z')
    return LABEL;
  if (c == '+' || c == '-' || c == '=')
    return BINOP;
  if (c == '"')
    return STRING;
  return NONE;
}

size_t tokenize(char const **input, char const **out, enum token_type *token_type_out) {
  while (**input == ' ') {
    ++*input;
  }

  if (!**input)
    return 0;

  *out = *input;
  *token_type_out = get_token_type(**input, NULL);
  if (*token_type_out == NONE)
    return 0;

  while (**input && get_token_type(**input, token_type_out) == *token_type_out) {
    if (*input != *out && **input == '"' && *token_type_out == STRING) {
      ++*input;
      break;
    }
    ++*input;
  }

  return *input - *out;
}
