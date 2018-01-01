#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <string.h>

#include "at_main.h"
#include "at_display.h"
#include "u8g2.h"

// for linker, emulator, and programmer's sake
#include <avr/avr_mcu_section.h>
AVR_MCU(F_CPU, "atmega328");

extern u8g2_t u8g2;

int main(void)
{
  init_display();

  char line[GETLINE_LEN + 1];

  putstr("AVR-BASIC\n");
  flush();

  while (1) {
    putstr(">");
    flush();
    int l = getline(line);
    line[l] = 0;

    
  }

  return 0;
}
