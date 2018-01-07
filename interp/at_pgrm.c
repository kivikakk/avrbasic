#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "at_pgrm.h"
#include "at_interp.h"

uint8_t PHEAP[0x400];

void add_line(uint16_t lno, char const *line, char *err) {
  size_t o = 0;

  // TODO:
  //
  // - [ ] Bad implementation (purely additive)
  // - [ ] Linked list implementation (store next/prev ptrs)

  // (uint16_t) line number
  // (uint8_t)  line length
  // (uint8_t*) line length * bytes

  int len = strlen(line);
  if (len > MAX_LINE_LEN) {
    snprintf(err, ERR_LEN, "%s", "line too long");
    return;
  }

  while (o + 3 + len < sizeof(PHEAP)) {
    if (*(uint16_t *)&PHEAP[o] == 0)
      break;
    o += 1 + PHEAP[2];
  }

  if (o + 3 + len >= sizeof(PHEAP)) {
    snprintf(err, ERR_LEN, "%s", "overran program heap");
    return;
  }

  *(uint16_t *)&PHEAP[o] = lno;
  PHEAP[2] = len;
  memcpy(&PHEAP[3], line, len);
}

int get_line(uint16_t lno, char line[MAX_LINE_LEN], char *err) {
  snprintf(err, ERR_LEN, "%s", "unimpl");
  return 0;
}
