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
  u8g2_Setup_st7920_p_128x64_1(&u8g2, U8G2_R0, u8x8_byte_8bit_8080mode, u8x8_gpio_and_delay);
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0);

  memset(DISPLAY, ' ', sizeof(DISPLAY));
}

static uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
      DDRB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3);
      DDRC = (1 << PC0) | (1 << PC1) | (1 << PC2);
      DDRD = (1 << PD2) | (1 << PD3) | (1 << PD5) | (1 << PD7);
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
        PORTC |= (1 << PC2);
      else
        PORTC &= ~(1 << PC2);
      break;

    case U8X8_MSG_GPIO_D1:				// D1 or SPI data pin: Output level in arg_int
      if (arg_int)
        PORTC |= (1 << PC1);
      else
        PORTC &= ~(1 << PC1);
      break;

    case U8X8_MSG_GPIO_D2:				// D2 pin: Output level in arg_int
      if (arg_int)
        PORTB |= (1 << PB3);
      else
        PORTB &= ~(1 << PB3);
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
        PORTB |= (1 << PB1);
      else
        PORTB &= ~(1 << PB1);
      break;

    case U8X8_MSG_GPIO_D6:				// D6 pin: Output level in arg_int
      if (arg_int)
        PORTB |= (1 << PB2);
      else
        PORTB &= ~(1 << PB2);
      break;

    case U8X8_MSG_GPIO_D7:				// D7 pin: Output level in arg_int
      if (arg_int)
        PORTC |= (1 << PC0);
      else
        PORTC &= ~(1 << PC0);
      break;

    case U8X8_MSG_GPIO_E:				// E/WR pin: Output level in arg_int
      if (arg_int)
        PORTD |= (1 << PD7);
      else
        PORTD &= ~(1 << PD7);
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

    default:
      u8x8_SetGPIOResult(u8x8, 1);			// default return value
      break;
  }
  return 1;
}

void flush(void)
{
  u8g2_FirstPage(&u8g2);
  do {
    u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);
    u8g2_SetDrawColor(&u8g2, 2);
    u8g2_SetFontMode(&u8g2, 1);

    for (int y = 0; y < H; ++y)
      for (int x = 0; x < W; ++x)
        u8g2_DrawGlyph(&u8g2, x * 6, y * 8 + 7, DISPLAY[y * W + x]);

    u8g2_DrawBox(&u8g2, X * 6, Y * 8, 6, 8);
  } while (u8g2_NextPage(&u8g2));
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

void putstrn(char const *s, size_t n)
{
  while (n--)
    putch(*s++);
}

void putstr(char const *s)
{
  while (*s)
    putch(*s++);
}

char getch(void)
{
  while (!(UCSR0A & (1 << RXC0)));
  char c = UDR0;
  if (c >= 'a' && c <= 'z') {
    c -= 'a' - 'A';
  }
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = c;
  return c;
}

int getln(char line[GETLN_LEN]) {
  int i = 0;

  while (1) {
    char c = getch();

    if ((c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == '(' ||
        c == ')' ||
        c == ' ' ||
        c == '%' ||
        c == '$' ||
        c == '*' ||
        c == '/' ||
        c == '+' ||
        c == '-' ||
        c == '"' ||
        c == '=') {
      if (i < GETLN_LEN) {
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

