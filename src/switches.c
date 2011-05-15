#include <stdint.h>
#include "yrdkrx62ndef.h"
#include "iodefine.h"


void switches_initialize(void)
{
  PORT4.DDR.BIT.B0 = 0;
  PORT4.DDR.BIT.B1 = 0;
  PORT4.DDR.BIT.B2 = 0;		

  PORT4.ICR.BIT.B0 = 1;
  PORT4.ICR.BIT.B1 = 1;
  PORT4.ICR.BIT.B2 = 1;
}


unsigned int switches_read(void)
{
  /* return the switch map */

#define SWITCHES_MASK ((1 << 2) | (1 << 1) | (1 << 0))

  uint8_t pre_state = 0;
  uint8_t redo_count = 0;
  uint8_t state_count;

  while (1)
  {
    /* dont do more than N passes */
    if ((++redo_count) == 3)
      goto on_done;

    pre_state = PORT4.PORT.BYTE & SWITCHES_MASK;

    /* consider stable as state_count reaches threshold */
    state_count = 0;
    while (pre_state == (PORT4.PORT.BYTE & SWITCHES_MASK))
    {
      if ((++state_count) == 3)
	goto on_done;
    }
  }

 on_done:
  /* invert the map */
  return ~pre_state;
}


/* debugging only */

#include "lcd.h"

void switches_schedule(void)
{
  unsigned int mask;
  unsigned int i;
  char buf[4];

  mask = switches_read();

  for (i = 0; i < 3; ++i)
    buf[i] = '0' + ((mask >> i) & 1);
  buf[i] = 0;

  lcd_string(6, 0, buf);
}
