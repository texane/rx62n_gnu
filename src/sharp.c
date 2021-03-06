#include "config.h"
#include "iodefine.h"
#include "sharp.h"
#include "adc.h"


void sharp_initialize_all(void)
{
  adc_initialize();
}


unsigned int sharp_adc10_to_mm(uint16_t v)
{
  /* find the voltage <= to v and return the distance, in mm */

  /* from experiments. TODO: binary search */
  static const unsigned int pairs[][2] =
  {
#define MV_TO_UINT16(__mv) (((__mv) * 0x3ff) / 3300)

    { MV_TO_UINT16(2860), 30 },
    { MV_TO_UINT16(2770), 40 },
    { MV_TO_UINT16(2340), 50 },
    { MV_TO_UINT16(1950), 60 },
    { MV_TO_UINT16(1680), 70 },
    { MV_TO_UINT16(1470), 80 },
    { MV_TO_UINT16(1320), 90 },
    { MV_TO_UINT16(1170), 100 },
    { MV_TO_UINT16(980), 120 },
    { MV_TO_UINT16(810), 140 },
    { MV_TO_UINT16(710), 160 },
    { MV_TO_UINT16(600), 180 },
    { MV_TO_UINT16(520), 200 },
    { MV_TO_UINT16(460), 220 },
    { MV_TO_UINT16(410), 250 },
    { MV_TO_UINT16(310), 300 }
  };

  static const unsigned int count = sizeof(pairs) / sizeof(pairs[0]);
  unsigned int pos;

  /* find the voltage */
  for (pos = 0; pos < count; ++pos)
    if (v > pairs[pos][0]) break ;

  /* pairs[pos - 1][0] < v < pairs[pos][0] */
  if (pos == 0) return 30;
  if (pos == count) return (unsigned int)-1;

  /* interpolate */
  unsigned int vd, dd;
  vd = pairs[pos - 1][0] - pairs[pos][0];
  dd = pairs[pos][1] - pairs[pos - 1][1];
  return pairs[pos - 1][1] + ((pairs[pos - 1][0] - v) * dd) / vd;
}


#if 0 /* UNUSED */

void sharp_schedule(void)
{
  static uint16_t values[2];
  static unsigned int chan = 0;
  static unsigned int rip = 0;

  /* read in progress */
  if (rip == 1)
  {
    values[chan] = adc_fini_read(chan);

    /* read done */
    if (values[chan] != (uint16_t)-1)
    {
      if (chan == 1)
      {
	lcd_string(6, 0, "adc ");
	lcd_uint16(6, 30, values[0]);
	lcd_uint16(6, 60, values[1]);

	lcd_string(7, 0, "mm  ");
	lcd_uint16(7, 30, sharp_adc10_to_mm(values[0]));
	lcd_uint16(7, 60, sharp_adc10_to_mm(values[1]));
      }

      chan = (chan + 1) & 1;

      rip = 0;
    }
  }
  else /* no read in progress */
  {
    static unsigned int prescal = 0;
    if ((++prescal) & (4 - 1)) return ;

    adc_init_read(chan);
    rip = 1;
  }
}

#endif /* UNUSED */
