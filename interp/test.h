#ifndef TEST_H
#define TEST_H

#ifdef TEST
#  define PROGMEM
#  define strcpy_P strcpy
#else
#  include <avr/pgmspace.h>
#endif

#endif
