#define __DELAY_BACKWARD_COMPATIBLE__

#define FOSC 1000000
#define BAUD 4800
#define MYUBRR FOSC/16/BAUD-1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <string.h>
#include <util/setbaud.h>

#include "at_main.h"

#include <avr/avr_mcu_section.h>
AVR_MCU(F_CPU, "atmega8");

int main(void)
{
  ADMUX |= (1 << REFS0) | (1 << MUX0) | (1 << MUX2);
  ADCSRA |= (1 << ADPS1) | (1 << ADPS0);
  ADCSRA |= (1 << ADEN);

  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
  UCSRA &= ~(1 << U2X);
  UCSRB = (1 << RXEN) | (1 << TXEN);
  UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);

  static char x = 0;

  while (1) {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC)) {}
    int r = ADC;

    char c = 0;
    if (r >= 600 && r <= 620) {
      c = 'A';
    } else if (r >= 690 && r <= 710) {
      c = 'B';
    } else if (r >= 845 && r <= 865) {
      c = 'C';
    } else if (r >= 920 && r <= 940) {
      c = 'D';
    } else if (r >= 1005 && r <= 1025) {
      c = 'E';
    } else {
      if (x) {
        _delay_ms(25);
      }
      x = 0;
    }

    if (c && c != x) {
      x = c;

      while (!(UCSRA & (1 << UDRE)));
      UDR = c;
    }
  }

  return 0;
}
