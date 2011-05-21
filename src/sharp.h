#ifndef SHARP_H_INCLUDED
# define SHARP_H_INCLUDED


#include <stdint.h>
#include "adc.h"


void sharp_initialize_all(void);
void sharp_schedule(void);
unsigned int sharp_adc10_to_mm(uint16_t);


/* innlined routines */

#define SHARP_ADC_INDEX_FR 0
#define SHARP_ADC_INDEX_FL 1

static inline unsigned int sharp_read(unsigned int index)
{
  return sharp_adc10_to_mm(adc_read(index));
}

static inline unsigned int sharp_read_fr(void)
{
  /* front right */
  return sharp_read(SHARP_ADC_INDEX_FR);
}

static inline unsigned int sharp_read_fl(void)
{
  /* front left */
  return sharp_read(SHARP_ADC_INDEX_FL);
}


#endif /* ! SHARP_H_INCLUDED */
