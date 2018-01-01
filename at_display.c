#define __DELAY_BACKWARD_COMPATIBLE__

#include "at_main.h"
#include "at_display.h"
#include "u8g2.h"

u8g2_t u8g2;

const int W = 21;
const int H = 6;

static uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

void init_display(void) {
  DDRB = (1 << PB0);
  DDRD = (1 << PD5) | (1 << PD6) | (1 << PD7);

  ADMUX |= (1 << REFS0);
  ADCSRA |= (1 << ADPS1) | (1 << ADPS0);
  ADCSRA |= (1 << ADEN);

  u8g2_Setup_st7920_s_128x64_f(&u8g2, U8G2_R0, u8x8_byte_4wire_sw_spi, u8x8_gpio_and_delay);
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0);
}

char getch(void)
{
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
      return c;
    }
  }
}

static uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
      // setup pins
      break;

    case U8X8_MSG_DELAY_NANO:
      _delay_us(0.001 * arg_int);
      break;

    case U8X8_MSG_DELAY_100NANO:
      _delay_us(0.1 * arg_int);
      break;

    case U8X8_MSG_DELAY_10MICRO:
      _delay_us(10 * arg_int);
      break;

    case U8X8_MSG_DELAY_MILLI:
      _delay_ms(arg_int);
      break;

    case U8X8_MSG_GPIO_D0:				// D0 or SPI clock pin: Output level in arg_int
      // E
      if (arg_int)
        PORTD |= (1 << PD7);
      else
        PORTD &= ~(1 << PD7);
      break;

    case U8X8_MSG_GPIO_D1:				// D1 or SPI data pin: Output level in arg_int
      // RW/Data/MOSI
      if (arg_int)
        PORTD |= (1 << PD6);
      else
        PORTD &= ~(1 << PD6);
      break;

    case U8X8_MSG_GPIO_D2:				// D2 pin: Output level in arg_int
      break;

    case U8X8_MSG_GPIO_D3:				// D3 pin: Output level in arg_int
      break;

    case U8X8_MSG_GPIO_D4:				// D4 pin: Output level in arg_int
      break;

    case U8X8_MSG_GPIO_D5:				// D5 pin: Output level in arg_int
      break;

    case U8X8_MSG_GPIO_D6:				// D6 pin: Output level in arg_int
      break;

    case U8X8_MSG_GPIO_D7:				// D7 pin: Output level in arg_int
      break;

    case U8X8_MSG_GPIO_E:				// E/WR pin: Output level in arg_int
      break;

    case U8X8_MSG_GPIO_CS:				// CS (chip select) pin: Output level in arg_int
      // RS/Chip Select
      if (arg_int)
        PORTD |= (1 << PD5);
      else
        PORTD &= ~(1 << PD5);
      break;

    case U8X8_MSG_GPIO_DC:				// DC (data/cmd, A0, register select) pin: Output level in arg_int
      break;

    case U8X8_MSG_GPIO_RESET:			// Reset pin: Output level in arg_int
      // RST/Reset
      if (arg_int)
        PORTB |= (1 << PB0);
      else
        PORTB &= ~(1 << PB0);
      break;

    case U8X8_MSG_GPIO_CS1:				// CS1 (chip select) pin: Output level in arg_int
      break;

    case U8X8_MSG_GPIO_CS2:				// CS2 (chip select) pin: Output level in arg_int
      break;

    default:
      u8x8_SetGPIOResult(u8x8, 1);			// default return value
      break;
  }
  return 1;
}
