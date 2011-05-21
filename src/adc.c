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
  /* an0, an1 input */
  /* an0, an1 buffer disabled */
  /* turn on and setup modules */

  SYSTEM.MSTPCRB.BIT.MSTPB0 = 0;

  PORT4.DDR.BIT.B0 = 0;
  PORT4.DDR.BIT.B1 = 0;
  PORT4.DDR.BIT.B2 = 0;
  PORT4.DDR.BIT.B3 = 0;

  PORT4.ICR.BIT.B0 = 1;
  PORT4.ICR.BIT.B1 = 1;
  PORT4.ICR.BIT.B2 = 1;
  PORT4.ICR.BIT.B3 = 1;

  MSTP_AD0 = 0;
  setup_adc(get_adc(0));

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
