#include <stdint.h>
#include "config.h"
#include "lcd.h"


static volatile uint32_t counter __attribute__((aligned));


void swatch_reset(void)
{
  counter = 0;
}

void swatch_initialize(void)
{
  swatch_reset();
}

void swatch_schedule(void)
{
  ++counter;
}

unsigned int swatch_get_msecs(void)
{
  return (counter * 1000) / CONFIG_TIMER_FREQ;
}

unsigned int swatch_is_game_over(void)
{
  return swatch_get_msecs() >= CONFIG_SWATCH_MSECS;
}
