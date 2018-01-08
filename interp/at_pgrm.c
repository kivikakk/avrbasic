#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "at_pgrm.h"
#include "at_interp.h"
#include "pgmspace.h"

uint8_t PHEAP[0x200];

char const LINE_NO_REQ_ERR[] PROGMEM = "line number required";
char const LINE_LENGTH_ERR[] PROGMEM = "line too long";
char const PHEAP_OVERRUN_ERR[] PROGMEM = "overran program heap";

uint16_t MIN_LINE, MAX_LINE;

struct pheap_entry {
  union {
    struct {
      unsigned last : 1;
      unsigned occupied : 1;
    };
    uint8_t _flags;
  };
  uint16_t length;
} __attribute__((__packed__));

struct pheap_entry_occupied {
  struct pheap_entry entry;
  uint16_t lno;
  uint8_t llen;
} __attribute__((__packed__));

void init_pheap(void) {
  struct pheap_entry *pheap = (struct pheap_entry *)PHEAP;
  pheap->occupied = 0;
  pheap->last = 1;
  pheap->length = 0x200 - sizeof(struct pheap_entry);
}

struct pheap_entry_occupied *find_line(uint16_t lno) {
  if (!MIN_LINE || lno < MIN_LINE || lno > MAX_LINE)
    return NULL;

  struct pheap_entry *pheap = (struct pheap_entry *)PHEAP;

  do {
    if (pheap->occupied) {
      struct pheap_entry_occupied *occupied = (struct pheap_entry_occupied *)pheap;
      if (occupied->lno == lno)
        return occupied;
    }

    pheap = (struct pheap_entry *)((uint8_t *)pheap + sizeof(struct pheap_entry) + pheap->length);
  } while (!pheap->last);

  return NULL;
}

struct pheap_entry *find_gap(uint16_t sz) {
  struct pheap_entry *pheap = (struct pheap_entry *)PHEAP;

  while (1) {
    if (!pheap->occupied && pheap->length >= sz)
      return pheap;
    if (pheap->last)
      break;
    pheap = (struct pheap_entry *)((uint8_t *)pheap + sizeof(struct pheap_entry) + pheap->length);
  };

  return NULL;
}

void remove_entry(struct pheap_entry *entry) {
  entry->occupied = 0;
  if (entry->last)
    return;

  struct pheap_entry *next = (struct pheap_entry *)((uint8_t *)entry + sizeof(struct pheap_entry) + entry->length);
  if (next->occupied)
    return;

  entry->last = next->last;
  entry->length += next->length + sizeof(struct pheap_entry);
}

void add_line(uint16_t lno, char const *line, char *err) {
  if (!lno) {
    strcpy_P(err, LINE_NO_REQ_ERR);
    return;
  }

  int len = strlen(line);
  if (len > MAX_LINE_LEN) {
    strcpy_P(err, LINE_LENGTH_ERR);
    return;
  }

  struct pheap_entry_occupied *extant, *target;

start:
  extant = find_line(lno);

  if (!extant) {
    struct pheap_entry *gap =
      find_gap(sizeof(struct pheap_entry_occupied) - sizeof(struct pheap_entry) + len);

    if (!gap) {
      strcpy_P(err, PHEAP_OVERRUN_ERR);
      return;
    }

    // can we subdivide this gap?
    if (gap->length - sizeof(struct pheap_entry_occupied) + sizeof(struct pheap_entry) > sizeof(struct pheap_entry_occupied)) {
      // we can fit at least a 1 character string in
      struct pheap_entry *ne = (struct pheap_entry *)((uint8_t *)gap + sizeof(struct pheap_entry_occupied) + len);
      ne->occupied = 0;
      ne->length = gap->length - sizeof(struct pheap_entry_occupied) + sizeof(struct pheap_entry) - len;
      ne->last = gap->last;
      gap->last = 0;
      gap->length = sizeof(struct pheap_entry_occupied) - sizeof(struct pheap_entry) + len;
    }

    target = (struct pheap_entry_occupied *)gap;
    target->entry.occupied = 1;
  } else {
    // can we fit in here?
    if (extant->entry.length - sizeof(struct pheap_entry_occupied) + sizeof(struct pheap_entry) >= len) {
      // yes.
      target = extant;
    } else {
      // no. deallocate, aggregate and try insert.
      remove_entry(&extant->entry);
      goto start;
    }
  }

  if (!MIN_LINE || lno < MIN_LINE)
    MIN_LINE = lno;

  if (!MAX_LINE || lno > MAX_LINE)
    MAX_LINE = lno;

  target->lno = lno;
  target->llen = len;
  memcpy(target + 1, line, len);
}

int get_line(uint16_t lno, char line[MAX_LINE_LEN], char *err) {
  struct pheap_entry_occupied *e = find_line(lno);
  if (!e)
    return 0;

  memcpy(line, e + 1, e->llen);
  return e->llen;
}
