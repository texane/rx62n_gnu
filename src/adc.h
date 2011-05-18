#ifndef ADC_H_INCLUDED
# define ADC_H_INCLUDED


#include <stdint.h>


void adc_initialize(void);
void adc_init_read(unsigned int);
uint16_t adc_fini_read(unsigned int);
uint16_t adc_read(unsigned int);
void adc_schedule(void);


#endif /* ADC_H_INCLUDED */
