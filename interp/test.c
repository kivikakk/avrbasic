#include <stdlib.h>
#include "at_interp.h"
#include "harness.h"

void token_type(test_batch_runner *runner) {
  INT_EQ(runner, get_token_type('A', NULL), LABEL, "get_token_type 'A'");
  enum token_type label = LABEL;
  INT_EQ(runner, get_token_type('A', &label), LABEL, "get_token_type 'A', LABEL");
  INT_EQ(runner, get_token_type('$', &label), LABEL, "get_token_type '$', LABEL");
  INT_EQ(runner, get_token_type('4', &label), NUMBER, "get_token_type '4', LABEL");
  INT_EQ(runner, get_token_type('=', &label), BINOP, "get_token_type '=', LABEL");
}

int main() {
  test_batch_runner *runner = test_batch_runner_new();

  token_type(runner);

  test_print_summary(runner);
  int retval = test_ok(runner) ? 0 : 1;
  free(runner);

  return retval;

  return 0;
}
