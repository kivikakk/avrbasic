#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "at_interp.h"
#include "harness.h"

char STDOUT_BUF[1024];
extern uint8_t VHEAP[0x400];

void putstr(char const *s) {
  strcat(STDOUT_BUF, s);
}

void test_token_type(test_batch_runner *runner) {
  INT_EQ(runner, get_token_type('A', NULL), T_LABEL, "get_token_type 'A'");
  enum token_type label = T_LABEL;
  INT_EQ(runner, get_token_type('A', &label), T_LABEL, "get_token_type 'A', T_LABEL");
  INT_EQ(runner, get_token_type('$', &label), T_LABEL, "get_token_type '$', T_LABEL");
  INT_EQ(runner, get_token_type('4', &label), T_NUMBER, "get_token_type '4', T_LABEL");
  INT_EQ(runner, get_token_type('=', &label), T_EQUAL, "get_token_type '=', T_LABEL");

  INT_EQ(runner, get_token_type('"', NULL), T_STRING, "get_token_type '\"'");
  enum token_type string = T_STRING;
  INT_EQ(runner, get_token_type('A', &string), T_STRING, "get_token_type 'A', T_STRING");
  INT_EQ(runner, get_token_type('"', &string), T_STRING, "get_token_type '\"', T_STRING");
}

void test_tokenize(test_batch_runner *runner) {
  char const *inp = "  LET X% = 1";

  char const *t = inp;
  char const *out;
  enum token_type type;
  INT_EQ(runner, tokenize(&t, &out, &type), 3, "tokenize 1.0");
  INT_EQ(runner, type, T_S_LET, "tokenize 1.1");
  STRN_EQ(runner, out, "LET", 3, "tokenize 1.2");

  INT_EQ(runner, tokenize(&t, &out, &type), 2, "tokenize 2.0");
  INT_EQ(runner, type, T_LABEL, "tokenize 2.1");
  STRN_EQ(runner, out, "X%", 2, "tokenize 2.2");

  INT_EQ(runner, tokenize(&t, &out, &type), 1, "tokenize 3.0");
  INT_EQ(runner, type, T_EQUAL, "tokenize 3.1");
  STRN_EQ(runner, out, "=", 1, "tokenize 3.2");

  INT_EQ(runner, tokenize(&t, &out, &type), 1, "tokenize 4.0");
  INT_EQ(runner, type, T_NUMBER, "tokenize 4.1");
  STRN_EQ(runner, out, "1", 1, "tokenize 4.2");

  INT_EQ(runner, tokenize(&t, &out, &type), 0, "tokenize 5.0");
}

void test_tokenize_string(test_batch_runner *runner) {
  char const *inp = "  LET X$ = \"A\"+\"B\"";

  char const *t = inp;
  char const *out;
  enum token_type type;
  INT_EQ(runner, tokenize(&t, &out, &type), 3, "tokenize_string 1.0");
  INT_EQ(runner, type, T_S_LET, "tokenize_string 1.1");
  STRN_EQ(runner, out, "LET", 3, "tokenize_string 1.2");

  INT_EQ(runner, tokenize(&t, &out, &type), 2, "tokenize_string 2.0");
  INT_EQ(runner, type, T_LABEL, "tokenize_string 2.1");
  STRN_EQ(runner, out, "X$", 2, "tokenize_string 2.2");

  INT_EQ(runner, tokenize(&t, &out, &type), 1, "tokenize_string 3.0");
  INT_EQ(runner, type, T_EQUAL, "tokenize_string 3.1");
  STRN_EQ(runner, out, "=", 1, "tokenize_string 3.2");

  INT_EQ(runner, tokenize(&t, &out, &type), 3, "tokenize_string 4.0");
  INT_EQ(runner, type, T_STRING, "tokenize_string 4.1");
  STRN_EQ(runner, out, "\"A\"", 3, "tokenize_string 4.2");

  INT_EQ(runner, tokenize(&t, &out, &type), 1, "tokenize_string 5.0");
  INT_EQ(runner, type, T_ADD, "tokenize_string 5.1");
  STRN_EQ(runner, out, "+", 1, "tokenize_string 5.2");

  INT_EQ(runner, tokenize(&t, &out, &type), 3, "tokenize_string 6.0");
  INT_EQ(runner, type, T_STRING, "tokenize_string 6.1");
  STRN_EQ(runner, out, "\"B\"", 3, "tokenize_string 6.2");

  INT_EQ(runner, tokenize(&t, &out, &type), 0, "tokenize_string 7.0");
}

void test_tokenize_empty(test_batch_runner *runner) {
  char const *inp = "";
  char const *t = inp, *out;
  enum token_type type;
  INT_EQ(runner, tokenize(&t, &out, &type), 0, "tokenize_empty");
}

void test_exec_expr(test_batch_runner *runner) {
  struct value value;
  char const *err = NULL;

  prep("1");
  value = exec_expr(&err);
  INT_EQ(runner, value.type, V_NUMBER, "exec_expr 1 value.type");
  INT_EQ(runner, value.as.number, 1, "exec_expr 1 value.as.number");

  prep("1 + 2");
  value = exec_expr(&err);
  INT_EQ(runner, value.type, V_NUMBER, "exec_expr 1 + 2 value.type");
  INT_EQ(runner, value.as.number, 3, "exec_expr 1 + 2 value.as.number");

  prep("1 - 2");
  value = exec_expr(&err);
  INT_EQ(runner, value.type, V_NUMBER, "exec_expr 1 - 2 value.type");
  INT_EQ(runner, value.as.number, -1, "exec_expr 1 - 2 value.as.number");

  prep("(1 - 2) - 2");
  value = exec_expr(&err);
  INT_EQ(runner, value.type, V_NUMBER, "exec_expr (1 - 2) - 2 value.type");
  INT_EQ(runner, value.as.number, -3, "exec_expr (1 - 2) - 2 value.as.number");

  prep("1 - 2 - 2");
  value = exec_expr(&err);
  INT_EQ(runner, value.type, V_NUMBER, "exec_expr 1 - 2 - 2 value.type");
  INT_EQ(runner, value.as.number, -3, "exec_expr 1 - 2 - 2 value.as.number");

  prep("1 - (2 - 2)");
  value = exec_expr(&err);
  INT_EQ(runner, value.type, V_NUMBER, "exec_expr 1 - (2 - 2) value.type");
  INT_EQ(runner, value.as.number, 1, "exec_expr 1 - (2 - 2) value.as.number");

  prep("1 - (2 - 2) = 0 + 2 - 1");
  value = exec_expr(&err);
  INT_EQ(runner, value.type, V_NUMBER, "exec_expr 1 - (2 - 2) = 0 + 2 - 1 value.type");
  INT_EQ(runner, value.as.number, 1, "exec_expr 1 - (2 - 2) = 0 + 2 - 1 value.as.number");

  prep("1 - (2 - 2) = 1 + 2 - 1");
  value = exec_expr(&err);
  INT_EQ(runner, value.type, V_NUMBER, "exec_expr 1 - (2 - 2) = 1 + 2 - 1 value.type");
  INT_EQ(runner, value.as.number, 0, "exec_expr 1 - (2 - 2) = 1 + 2 - 1 value.as.number");

  INT_EQ(runner, (int)err, 0, "no errors");
  prep("1 -");
  value = exec_expr(&err);
  STR_EQ(runner, err, "expected factor, label or LPAREN", "exec_expr 1 - error");
}

void test_exec_stmt_print(test_batch_runner *runner) {
  char const *err = NULL;

  STDOUT_BUF[0] = 0;
  exec_stmt("PRINT 4+4*2", &err);
  STR_EQ(runner, err, NULL, "PRINT success");
  STR_EQ(runner, STDOUT_BUF, "12\n", "PRINT result");
}

void test_exec_stmt_let(test_batch_runner *runner) {
  char const *err = NULL;
  exec_stmt("LET", &err);
  STR_EQ(runner, err, "unexpected symbol T_NONE, expected T_LABEL", "LET wants args");

  err = NULL;
  exec_stmt("LET XYZ = 1", &err);
  STR_EQ(runner, err, "expected var name to end in % or $", "LET wants typed var name");

  err = NULL;
  exec_stmt("LET XYZ% = 5", &err);
  STR_EQ(runner, err, NULL, "LET success");

  STDOUT_BUF[0] = 0;
  exec_stmt("PRINT XYZ% * 2", &err);
  STR_EQ(runner, err, NULL, "LET then PRINT success");
  STR_EQ(runner, STDOUT_BUF, "10\n", "LET then PRINT result");
}


int main() {
  test_batch_runner *runner = test_batch_runner_new();

  test_token_type(runner);
  test_tokenize(runner);
  test_tokenize_string(runner);
  test_exec_expr(runner);
  test_exec_stmt_print(runner);
  test_exec_stmt_let(runner);

  test_print_summary(runner);
  int retval = test_ok(runner) ? 0 : 1;
  free(runner);

  return retval;

  return 0;
}
