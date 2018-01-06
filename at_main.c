#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <string.h>
#define BAUD 9600
#include <util/setbaud.h>

#include "at_main.h"
#include "at_display.h"
#include "at_exec.h"
#include "u8g2.h"

#include <avr/avr_mcu_section.h>
AVR_MCU(F_CPU, "atmega328");

extern u8g2_t u8g2;

int main(void)
{
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;
  UCSR0A &= ~(1 << U2X0);
  UCSR0B = (1 << RXEN0);
  UCSR0C = (1 << UMSEL00) | (1 << UCSZ01) | (1 << UCSZ00);
  DDRD &= ~(1 << PD4);

  init_display();

  char line[GETLINE_LEN + 1];

  putstr("AVR-BASIC\n");
  flush();

  while (1) {
    putstr(">");
    flush();
    int l = getline(line);
    line[l] = 0;

    exec(line);
  }

  return 0;
}
