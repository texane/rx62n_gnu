#include <stdint.h>
#include "config.h"
#include "lcd.h"


static uint32_t countdown;
static uint32_t countstep;


void swatch_task_initialize(void)
{
  countdown = (CONFIG_SWATCH_MSECS * CONFIG_TIMER_FREQ) / 1000;
}

void swatch_task_schedule(void)
{
  if (countdown == 0)
  {
    /* TODO */
    lcd_string(5, 0, "game_is_over");
    return ;
  }

  --countdown;
}
