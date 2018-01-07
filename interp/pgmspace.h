#ifndef PGMSPACE_H
#define PGMSPACE_H

#ifdef TEST
#  define PROGMEM
#  define strcpy_P strcpy
#else
#  include <avr/pgmspace.h>
#endif

#endif
