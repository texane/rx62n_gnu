#ifndef SHARP_H_INCLUDED
# define SHARP_H_INCLUDED


#include <stdint.h>
#include "adc.h"


void sharp_initialize_all(void);
unsigned int sharp_adc10_to_mm(uint16_t);

#if 0 /* UNUSED */
void sharp_schedule(void);
#endif /* UNUSED */


/* innlined routines */

#define SHARP_ADC_INDEX_FL 0 /* front left */
#define SHARP_ADC_INDEX_FR 1 /* front right */
#define SHARP_ADC_INDEX_FM 2 /* front middle */
#define SHARP_ADC_INDEX_LB 11 /* left back */
#define SHARP_ADC_INDEX_LF 12 /* left front */

#define SHARP_ADC_INDEX_RB 11 /* right back */
#define SHARP_ADC_INDEX_RF 12 /* right front */

#include "igreboard.h"

extern igreboard_dev_t igreboard_device;

static inline unsigned int sharp_read(unsigned int index)
{
#if 0 /* local adc */
  return sharp_adc10_to_mm(adc_read(index));
#else /* igreboard adc */
  unsigned int value;
  igreboard_read_adc(&igreboard_device, index, &value);
  return sharp_adc10_to_mm(value);
#endif
}

static inline unsigned int sharp_read_2(unsigned int index)
{
#if 0 /* local adc */
  return sharp_adc10_to_mm(adc_read(index));
#else /* igreboard adc */
  unsigned int value;
  igreboard_read_adc_2(&igreboard_device, index, &value);
  return sharp_adc10_to_mm(value);
#endif
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

static inline unsigned int sharp_read_fm(void)
{
  /* front middle */
  return sharp_read(SHARP_ADC_INDEX_FM);
}

static inline unsigned int sharp_read_lh(void)
{
  /* left high */
  return sharp_read(8);
}

static inline unsigned int sharp_read_lb(void)
{
  /* left back, rb11 */
  return sharp_read(SHARP_ADC_INDEX_LB);
}

static inline unsigned int sharp_read_lf(void)
{
  /* left front, rb12 */
  return sharp_read(SHARP_ADC_INDEX_LF);
}

static inline unsigned int sharp_read_rh(void)
{
  /* right high */
  return sharp_read(9);
}

static inline unsigned int sharp_read_rb(void)
{
  /* right front, rb12 */
  return sharp_read_2(SHARP_ADC_INDEX_RB);
}

static inline unsigned int sharp_read_rf(void)
{
  /* right front, rb11 */
  return sharp_read_2(SHARP_ADC_INDEX_RF);
}

static inline unsigned int sharp_read_b(void)
{
  /* back */
  return 0;
}


#endif /* ! SHARP_H_INCLUDED */
