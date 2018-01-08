#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "at_interp.h"
#include "at_pgrm.h"

void putch(char c) {
  putc(c, stdout);
  fflush(stdout);
}

void putstr(char *s) {
  while (*s)
    putch(*s++);
}

int getln(char line[GETLN_LEN]) {
  char *l = NULL;
  size_t sz = 0;
  ssize_t r = getline(&l, &sz, stdin);
  if (r == -1) {
    putch('\n');
    free(l);
    exit(0);
  }
  if (r > GETLN_LEN)
    r = GETLN_LEN;
  if (r && l[r - 1] == '\n')
    --r;
  memcpy(line, l, r);
  for (size_t i = 0; i < r; ++i)
    line[i] = toupper(line[i]);
  free(l);
  return r;
}

void flush(void) {}

int main(int argc, char **argv) {
  char line[GETLN_LEN + 1];
  init_pheap();

  if (argc > 1) {
    FILE *f = fopen(argv[1], "r");
    ssize_t r;
    char *buf = NULL;
    size_t buf_sz = 0;
    char err[ERR_LEN];


    while ((r = getline(&buf, &buf_sz, f))) {
      if (r < 1)
        break;
      if (r > GETLN_LEN)
        r = GETLN_LEN;
      if (buf[r - 1] == '\n')
        --r;
      memcpy(line, buf, r);
      for (size_t i = 0; i < r; ++i)
        line[i] = toupper(line[i]);

      *err = 0;
      exec_stmt(line, err);
      if (*err) {
        putstr("ERR: ");
        putstr(err);
        putstr("\n");
        fclose(f);
        free(buf);
        return 1;
      }
    }

    fclose(f);
    free(buf);

    exec_stmt("RUN", err);

    return 0;
  }

  printf("AVR-BASIC\n");

  while (1) {
    putstr(">");
    int l = getln(line);
    line[l] = 0;

    char err[ERR_LEN];
    *err = 0;
    exec_stmt(line, err);
    if (*err) {
      putstr("ERR: ");
      putstr(err);
      putstr("\n");
    }
  }

  return 0;
}
