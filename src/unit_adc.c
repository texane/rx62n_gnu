#include "config.h"
#include <stdint.h>
#include "lcd.h"
#include "swatch.h"
#include "sharp.h"


static inline char nibble_to_ascii(uint8_t value)
{
  if (value >= 0xa) return 'a' + value - 0xa;
  return '0' + value;
}

static const char* uint16_to_string(uint16_t value)
{
  static char buf[8];
  unsigned int i;

  for (i = 0; i < 4; ++i, value >>= 4)
    buf[4 - i - 1] = nibble_to_ascii(value & 0xf);
  buf[i] = 0;

  return buf;
}

static inline void print_uint16
(unsigned int row, unsigned int col, uint16_t value)
{
  lcd_string((uint8_t)row, (uint8_t)col, uint16_to_string(value));
}

void unit_adc(void)
{
  unsigned int values[2];
  unsigned int i;
  unsigned int row, col;
  unsigned int msecs[2];

  msecs[1] = swatch_get_elapsed_msecs();

  while (1)
  {
    /* wait for at least 1 second */
    msecs[0] = swatch_get_elapsed_msecs();
    if ((msecs[0] - msecs[1]) < 1000) continue ;
    msecs[1] = msecs[0];

    lcd_string(0, 0, "barfu");
    values[0] = sharp_read_fl();
    values[1] = sharp_read_fr();

    /* display adcs */
    row = 0;
    for (i = 0; i < 2; ++i)
    {
      col = 30;
      if ((i & 1) == 0)
      {
	++row;
	col = 0;
      }

      print_uint16(row, col, (uint16_t)values[i]);
    }
  }
}
