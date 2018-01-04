#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <string.h>
#define BAUD 9600
#include <util/setbaud.h>

#include "at_main.h"

#include <avr/avr_mcu_section.h>
AVR_MCU(F_CPU, "atmega8");

int main(void)
{
  //ADMUX |= (1 << REFS0) | (1 << MUX0) | (1 << MUX2);
  //ADCSRA |= (1 << ADPS1) | (1 << ADPS0);
  //ADCSRA |= (1 << ADEN);

  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
  UCSRA &= ~(1 << U2X);
  UCSRB = (1 << TXEN) | (1 << RXEN);
  UCSRC = (1 << UCSZ1) | (1 << UCSZ0);

  while (1) {
    while (!(UCSRA & (1 << UDRE)));
    UDR = 'H';
  }

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
      c = '\n';
    } else if (r >= 1005 && r <= 1025) {
      c = 8;
    } else {
      x = 0;
    }

    if (c && c != x) {
      x = c;

      SPDR = c;
      while (!(SPSR & (1 << SPIF)));
    }
  }

  return 0;
}
