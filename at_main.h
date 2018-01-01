#ifndef AT_MAIN_H
#define AT_MAIN_H

#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>

#include <avr/pgmspace.h>

#define printf(format, ...) printf_P(PSTR(format), ## __VA_ARGS__)
#define sprintf(wh, format, ...) sprintf_P(wh, PSTR(format), ## __VA_ARGS__)

#endif
