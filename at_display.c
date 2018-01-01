#define __DELAY_BACKWARD_COMPATIBLE__

#include "at_main.h"
#include "at_display.h"
#include "u8g2.h"

#include <string.h>

u8g2_t u8g2;

#define W 21
#define H 8
static char DISPLAY[W * H];
static int X = 0;
static int Y = 0;

static uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

void init_display(void) {
  DDRB = (1 << PB0) | (1 << PB1) | (1 << PB6) | (1 << PB7);
  DDRD = (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD7);

  ADMUX |= (1 << REFS0);
  ADCSRA |= (1 << ADPS1) | (1 << ADPS0);
  ADCSRA |= (1 << ADEN);

  u8g2_Setup_st7920_p_128x64_f(&u8g2, U8G2_R0, u8x8_byte_8bit_8080mode, u8x8_gpio_and_delay);
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0);

  memset(DISPLAY, ' ', sizeof(DISPLAY));
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
      if (arg_int)
        PORTB |= (1 << PB7);
      else
        PORTB &= ~(1 << PB7);
      break;

    case U8X8_MSG_GPIO_D1:				// D1 or SPI data pin: Output level in arg_int
      if (arg_int)
        PORTB |= (1 << PB6);
      else
        PORTB &= ~(1 << PB6);
      break;

    case U8X8_MSG_GPIO_D2:				// D2 pin: Output level in arg_int
      if (arg_int)
        PORTD |= (1 << PD4);
      else
        PORTD &= ~(1 << PD4);
      break;

    case U8X8_MSG_GPIO_D3:				// D3 pin: Output level in arg_int
      if (arg_int)
        PORTD |= (1 << PD3);
      else
        PORTD &= ~(1 << PD3);
      break;

    case U8X8_MSG_GPIO_D4:				// D4 pin: Output level in arg_int
      if (arg_int)
        PORTD |= (1 << PD2);
      else
        PORTD &= ~(1 << PD2);
      break;

    case U8X8_MSG_GPIO_D5:				// D5 pin: Output level in arg_int
      if (arg_int)
        PORTD |= (1 << PD1);
      else
        PORTD &= ~(1 << PD1);
      break;

    case U8X8_MSG_GPIO_D6:				// D6 pin: Output level in arg_int
      if (arg_int)
        PORTD |= (1 << PD0);
      else
        PORTD &= ~(1 << PD0);
      break;

    case U8X8_MSG_GPIO_D7:				// D7 pin: Output level in arg_int
      if (arg_int)
        PORTB |= (1 << PB1);
      else
        PORTB &= ~(1 << PB1);
      break;

    case U8X8_MSG_GPIO_E:				// E/WR pin: Output level in arg_int
      if (arg_int)
        PORTD |= (1 << PD7);
      else
        PORTD &= ~(1 << PD7);
      break;

  case U8X8_MSG_GPIO_CS:				// CS = RW? (chip select) pin: Output level in arg_int// RW/Data/MOSI
      break;

    case U8X8_MSG_GPIO_DC:				// DC = RS(CS*) (data/cmd, A0, register select) pin: Output level in arg_int
      // RS/Chip Select
      if (arg_int)
        PORTD |= (1 << PD5);
      else
        PORTD &= ~(1 << PD5);
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

void flush(void)
{
  u8g2_ClearBuffer(&u8g2);
  u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);
  u8g2_SetDrawColor(&u8g2, 2);
  u8g2_SetFontMode(&u8g2, 1);

  for (int y = 0; y < H; ++y) {
    for (int x = 0; x < W; ++x) {
      u8g2_DrawGlyph(&u8g2, x * 6, y * 8 + 7, DISPLAY[y * W + x]);
    }
  }
  u8g2_DrawBox(&u8g2, X * 6, Y * 8, 6, 8);

  u8g2_SendBuffer(&u8g2);
}

void scroll(void)
{
  if (Y == H) {
    Y -= 1;
    for (int y = 0; y < H - 1; ++y)
      memcpy(&DISPLAY[y * W], &DISPLAY[(y + 1) * W], W);
    memset(&DISPLAY[(H - 1) * W], ' ', W);
  }
}

void putch(char c)
{
  if (c == 10) {
    if (Y < H - 1) {
      Y += 1;
      X = 0;
    } else {
      Y += 1;
      X = 0;
      scroll();
    }
  } else if (c == 8) {
    if (X > 0) {
      X -= 1;
    }
  } else {
    DISPLAY[Y * W + X] = c;
    if (X == W - 1) {
      Y += 1;
      X = 0;
      scroll();
    } else {
      X++;
    }
  }
}

void putstr(char const *s)
{
  while (*s)
    putch(*s++);
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

int getline(char line[GETLINE_LEN]) {
  int i = 0;

  while (1) {
    char c = getch();

    if ((c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == ' ' ||
        c == '%' ||
        c == '$' ||
        c == '*' ||
        c == '/' ||
        c == '+' ||
        c == '-' ||
        c == '"' ||
        c == '=') {
      if (i < GETLINE_LEN) {
        putch(c);
        flush();
        line[i] = c;
        ++i;
      }
    } else if (c == 8) {
      if (i > 0) {
        putstr("\b \b");
        flush();
        --i;
      }
    } else if (c == 10) {
      putch('\n');
      flush();
      return i;
    }
  }
}

