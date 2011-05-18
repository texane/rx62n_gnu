#include "config.h"
#include <stdint.h>
#include "iodefine.h"



/* get the ADC struct by index */

static inline volatile struct st_ad* get_adc(unsigned int i)
{
  return (i == 0) ? &AD0 : &AD1;
}


/* adc initialization */

static void setup_adc(volatile struct st_ad* ad)
{
  /* setup a 10 bit adc */

  ad->ADCSR.BIT.ADST = 0;

  ad->ADCSR.BYTE = 0;
  ad->ADCR.BYTE = 0;
  ad->ADDPR.BIT.DPSEL = 0;
}

void adc_initialize(void)
{
  /* an0, an2 input */
  /* an0, an2 buffer disabled */
  /* turn on and setup modules */

  SYSTEM.MSTPCRB.BIT.MSTPB0 = 0;

  PORT4.DDR.BIT.B0 = 0;
  PORT4.ICR.BIT.B0 = 0;
  MSTP_AD0 = 0;
  setup_adc(get_adc(0));

  PORT4.DDR.BIT.B2 = 0;
  PORT4.ICR.BIT.B2 = 0;
  MSTP_AD1 = 0;
  setup_adc(get_adc(1));
}


/* read an adc value */

void adc_init_read(unsigned int index)
{
  /* index the channel index, 0 <= index < 8 */
  volatile struct st_ad* const ad = get_adc(index >= 4 ? 1 : 0);

  /* select the channel */
  ad->ADCSR.BIT.CH = index;

  /* start the operation */
  ad->ADCSR.BIT.ADST = 1;
}

uint16_t adc_fini_read(unsigned int index)
{
  /* index the channel index, 0 <= index < 8 */
  /* return (uint16_t)-1 if conversion not yet ready */

  volatile struct st_ad* const ad = get_adc(index >= 4 ? 1 : 0);

  uint16_t value = (uint16_t)-1;

  /* previous operation is done */
  if (ad->ADCSR.BIT.ADST == 0)
  {
    switch (index & 3)
    {
    default:
    case 0: value = ad->ADDRA; break ;
    case 1: value = ad->ADDRB; break ;
    case 2: value = ad->ADDRC; break ;
    case 3: value = ad->ADDRD; break ;
    }

    value &= 0x3ff;
  }

  return  value;
}

uint16_t adc_read(unsigned int index)
{
  uint16_t value = (uint16_t)-1;

  adc_init_read(index);

  while (value == (uint16_t)-1)
    value = adc_fini_read(index);

  return value;
}


/* toremove */

#include "lcd.h"

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
  lcd_string(row, col, uint16_to_string(value));
}

void adc_schedule(void)
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
	lcd_string(7, 0, "adc ");
	print_uint16(7, 30, values[0]);
	print_uint16(7, 60, values[1]);
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
