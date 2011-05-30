#include "config.h"
#include <stdint.h>
#include "lcd.h"
#include "swatch.h"
#include "sharp.h"

void unit_adc(void)
{
  unsigned int values[2];
  unsigned int i;
  unsigned int row, col;
  unsigned int msecs[2];

  msecs[1] = swatch_get_msecs();

  while (1)
  {
    /* wait for at least 1 second */
    msecs[0] = swatch_get_msecs();
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

      lcd_uint16(row, col, (uint16_t)values[i]);
    }
  }
}
