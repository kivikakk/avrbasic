#ifndef PGMSPACE_H
#define PGMSPACE_H

#ifdef TEST
#  define PROGMEM
#  define PGM_P char const *
#  define strcpy_P strcpy
#  define strcat_P strcat
#else
#  include <avr/pgmspace.h>
#endif

#endif
