#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <string.h>
#define BAUD 4800
#include <util/setbaud.h>

#include "at_main.h"
#include "at_display.h"
#include "interp/at_interp.h"
#include "u8g2.h"

#include <avr/avr_mcu_section.h>
AVR_MCU(F_CPU, "atmega328");

extern u8g2_t u8g2;

int main(void)
{
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;
  UCSR0A &= ~(1 << U2X0);
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

  init_display();

  char line[GETLN_LEN + 1];

  putstr("AVR-BASIC\n");
  flush();

  while (1) {
    putstr(">");
    flush();
    int l = getln(line);
    line[l] = 0;

    char err[ERR_LEN];
    *err = 0;
    exec_stmt(line, err);
    if (*err) {
      putstr("ERR: ");
      putstr(err);
      putstr("\n");
    }
  }

  return 0;
}
