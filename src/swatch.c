#include <stdint.h>
#include "config.h"
#include "lcd.h"


static volatile uint32_t counter __attribute__((aligned));
static uint32_t start_msecs = (uint32_t)-1;

void swatch_initialize(void)
{
  counter = 0;
}

void swatch_schedule(void)
{
  ++counter;
}

unsigned int swatch_get_msecs(void)
{
  return (counter * 1000) / CONFIG_TIMER_FREQ;
}


unsigned int swatch_get_game_msecs(void)
{
  if (start_msecs == (uint32_t)-1) return 0;
  return swatch_get_msecs() - start_msecs;
}

unsigned int swatch_is_game_over(void)
{
  return swatch_get_game_msecs() >= CONFIG_SWATCH_MSECS;
}

void swatch_start_game(void)
{
  start_msecs = swatch_get_msecs();
}

unsigned int swatch_wait_msecs(unsigned int msecs)
{
  const unsigned int prev_msecs = swatch_get_msecs();
  unsigned int cur_msecs = 0;
  while (1)
  {
    cur_msecs = swatch_get_msecs();
    if ((cur_msecs - prev_msecs) >= msecs) break ;
  }

  return cur_msecs;
}
