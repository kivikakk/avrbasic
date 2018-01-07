#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "at_pgrm.h"
#include "at_interp.h"
#include "pgmspace.h"

uint8_t PHEAP[0x400];

char const LINE_NO_REQ_ERR[] PROGMEM = "line number required";
char const LINE_LENGTH_ERR[] PROGMEM = "line too long";
char const PHEAP_OVERRUN_ERR[] PROGMEM = "overran program heap";

static uint16_t MIN_LINE, MAX_LINE;

void add_line(uint16_t lno, char const *line, char *err) {
  size_t o = 0;

  // TODO:
  //
  // - [ ] Bad implementation (purely additive)
  // - [ ] Linked list implementation (store next/prev ptrs)

  // (uint16_t) line number
  // (uint8_t)  line length
  // (uint8_t*) line length * bytes

  if (!lno) {
    strcpy_P(err, LINE_NO_REQ_ERR);
    return;
  }

  int len = strlen(line);
  if (len > MAX_LINE_LEN) {
    strcpy_P(err, LINE_LENGTH_ERR);
    return;
  }

  while (o + 3 + len < sizeof(PHEAP)) {
    if (*(uint16_t *)&PHEAP[o] == 0)
      break;
    o += 3 + PHEAP[o + 2];
  }

  if (o + 3 + len >= sizeof(PHEAP)) {
    strcpy_P(err, PHEAP_OVERRUN_ERR);
    return;
  }

  if (!MIN_LINE || lno < MIN_LINE)
    MIN_LINE = lno;

  if (!MAX_LINE || lno > MAX_LINE)
    MAX_LINE = lno;

  *(uint16_t *)&PHEAP[o] = lno;
  PHEAP[o + 2] = len;
  memcpy(&PHEAP[o + 3], line, len);
}

int get_line(uint16_t lno, char line[MAX_LINE_LEN], char *err) {
  size_t o = 0, p = -1;

  while (o + 3 < sizeof(PHEAP)) {
    if (*(uint16_t *)&PHEAP[o] == lno)
      p = o;
    if (*(uint16_t *)&PHEAP[o] == 0)
      break;
    o += 3 + PHEAP[o + 2];
  }

  if (p == -1)
    return 0;

  int len = PHEAP[p + 2];
  memcpy(line, &PHEAP[p + 3], len);
  return len;
}
