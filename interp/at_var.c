#include <stdio.h>
#include <string.h>
#include "at_var.h"

uint8_t VHEAP[0x200];

void add_var(char name[3], struct value v, char const **err) {
  size_t o = 0;

  while (o < sizeof(VHEAP) - 5) {
    if (VHEAP[o] == 0 ||
        (VHEAP[o] == name[0] &&
         VHEAP[o+1] == name[1] &&
         VHEAP[o+2] == name[2])) {
      VHEAP[o] = name[0];
      VHEAP[o+1] = name[1];
      VHEAP[o+2] = name[2];
      if (name[2] == '%') {
        *(int16_t *)&VHEAP[o+3] = v.as.number;
      } else if (name[2] == '$') {
        VHEAP[o+3] = strlen(v.as.string);
        memcpy(&VHEAP[o+4], v.as.string, VHEAP[o+3]);
      } else {
        *err = "add_var encountered unknown var";
      }
      return;
    } else if (VHEAP[o + 2] == '%') {
      o += 5;
    } else if (VHEAP[o + 2] == '$') {
      o += 4 + MAX_STRING;
    } else {
      *err = "add_var encountered unknown var";
      return;
    }
  }

  *err = "out of vheap space";
}

struct value get_var(char name[3], char const **err) {
  struct value v;
  v.type = V_NUMBER;
  v.as.number = -32768;
  size_t o = 0;

  while (o < sizeof(VHEAP) - 5) {
    if (VHEAP[o] == 0) {
      static char GET_VAR_ERR[14];
      snprintf(GET_VAR_ERR, sizeof(GET_VAR_ERR), "%c%c%c undefined",
               name[0],
               name[1] ? name[1] : ' ',
               name[2]);
      *err = GET_VAR_ERR;
      return v;
    } else if (VHEAP[o] == name[0] &&
               VHEAP[o+1] == name[1] &&
               VHEAP[o+2] == name[2]) {
      if (name[2] == '%') {
        v.as.number = *(int16_t *)(&VHEAP[o + 3]);
      } else if (name[2] == '$') {
        v.type = V_STRING;
        int len = VHEAP[o+3];
        memcpy(v.as.string, &VHEAP[o+4], len);
        v.as.string[len] = 0;
      } else {
        *err = "get_var encountered unknown var";
      }
      return v;
    } else if (VHEAP[o + 2] == '%') {
      o += 5;
    } else if (VHEAP[o + 2] == '$') {
      o += 4 + MAX_STRING;
    } else {
      *err = "get_var encountered unknown var";
      return v;
    }
  }

  return v;
}
