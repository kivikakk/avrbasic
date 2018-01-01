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
  ADMUX |= (1 << REFS0);
  ADCSRA |= (1 << ADPS1) | (1 << ADPS0);
  ADCSRA |= (1 << ADEN);

  init_display();

  char c = 'A';

  while (1) {
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);
    u8g2_SetDrawColor(&u8g2, 2);
    u8g2_SetFontMode(&u8g2, 1);

    u8g2_DrawStr(&u8g2, 0 * 6, 0 * 8 + 7, "AVR-BASIC");
    u8g2_DrawGlyph(&u8g2, 0, 16, c);

    u8g2_DrawBox(&u8g2, 0 * 6, 1 * 8, 6, 8);

    u8g2_SendBuffer(&u8g2);

    c = getch();

  }

  return 0;
}
