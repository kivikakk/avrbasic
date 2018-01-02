#include <stdlib.h>
#include "at_interp.h"
#include "harness.h"

void test_token_type(test_batch_runner *runner) {
  INT_EQ(runner, get_token_type('A', NULL), LABEL, "get_token_type 'A'");
  enum token_type label = LABEL;
  INT_EQ(runner, get_token_type('A', &label), LABEL, "get_token_type 'A', LABEL");
  INT_EQ(runner, get_token_type('$', &label), LABEL, "get_token_type '$', LABEL");
  INT_EQ(runner, get_token_type('4', &label), NUMBER, "get_token_type '4', LABEL");
  INT_EQ(runner, get_token_type('=', &label), BINOP, "get_token_type '=', LABEL");

  INT_EQ(runner, get_token_type('"', NULL), STRING, "get_token_type '\"'");
  enum token_type string = STRING;
  INT_EQ(runner, get_token_type('A', &string), STRING, "get_token_type 'A', STRING");
  INT_EQ(runner, get_token_type('"', &string), STRING, "get_token_type '\"', STRING");
}

void test_tokenize(test_batch_runner *runner) {
  char const *inp = "  LET X% = 1";

  char const *t = inp;
  char const *out;
  enum token_type type;
  INT_EQ(runner, tokenize(&t, &out, &type), 3, "tokenize 1.0");
  INT_EQ(runner, type, LABEL, "tokenize 1.1");
  STRN_EQ(runner, out, "LET", 3, "tokenize 1.2");

  INT_EQ(runner, tokenize(&t, &out, &type), 2, "tokenize 2.0");
  INT_EQ(runner, type, LABEL, "tokenize 2.1");
  STRN_EQ(runner, out, "X%", 2, "tokenize 2.2");

  INT_EQ(runner, tokenize(&t, &out, &type), 1, "tokenize 3.0");
  INT_EQ(runner, type, BINOP, "tokenize 3.1");
  STRN_EQ(runner, out, "=", 1, "tokenize 3.2");

  INT_EQ(runner, tokenize(&t, &out, &type), 1, "tokenize 4.0");
  INT_EQ(runner, type, NUMBER, "tokenize 4.1");
  STRN_EQ(runner, out, "1", 1, "tokenize 4.2");
}

int main() {
  test_batch_runner *runner = test_batch_runner_new();

  test_token_type(runner);
  test_tokenize(runner);

  test_print_summary(runner);
  int retval = test_ok(runner) ? 0 : 1;
  free(runner);

  return retval;

  return 0;
}
