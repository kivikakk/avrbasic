#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <string.h>

#include "atmega328_arg.h"
#include "u8g2.h"

// for linker, emulator, and programmer's sake
#include <avr/avr_mcu_section.h>
AVR_MCU(F_CPU, "atmega328");

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
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

static u8g2_t u8g2;

void prep_display(void);
void draw_str(unsigned char x, unsigned char y, char const *str);
void send_display(void);

void init_st7920(void)
{
  DDRB = (1 << PB0);
  DDRD = (1 << PD5) | (1 << PD6) | (1 << PD7);

  u8g2_Setup_st7920_s_128x64_f(&u8g2, U8G2_R0, u8x8_byte_4wire_sw_spi, u8x8_gpio_and_delay);
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0);
}

void prep_display(void)
{
  u8g2_ClearBuffer(&u8g2);
  u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);
  u8g2_SetDrawColor(&u8g2, 2);
  u8g2_SetFontMode(&u8g2, 1);
}

void draw_str(unsigned char x, unsigned char y, char const *str)
{
  u8g2_DrawStr(&u8g2, x * 6, y * 8 + 7, str);
}

void draw_strn(unsigned char x, unsigned char y, char *str, unsigned char off, unsigned char n)
{
  str = str + off;
  char c = str[n];
  str[n] = 0;
  draw_str(x, y, str);
  str[n] = c;
}

void draw_cursor(unsigned char x, unsigned char y)
{
  u8g2_DrawBox(&u8g2, x * 6, y * 8, 6, 8);
}

void send_display(void)
{
  u8g2_SendBuffer(&u8g2);
}
