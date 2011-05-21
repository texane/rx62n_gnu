#include "config.h"
#include <stdint.h>
#if CONFIG_ENABLE_AVERSIVE
# include "aversive.h"
#endif



#define RADAR_STATE_SCAN (1 << 0)
#define RADAR_STATE_BLOCK (1 << 1)
static volatile uint32_t state;

static volatile uint32_t blocked_ticks;


void radar_initialize(void)
{
#if 0 /* not implemented */
  state = RADAR_STATE_SCAN;
  blocked_count = 0;
#endif
}


void radar_enable_block(void)
{
}


void radar_disable_block(void)
{
}

void radar_enable_scan(void)
{
  state |= RADAR_STATE_SCAN;
}

void radar_disable_scan(void)
{
  state &= ~RADAR_STATE_SCAN;
}


void radar_schedule(void)
{
#if 0 /* not implemented */

#define CONFIG_RADAR_BLOCK_TICKS

  if (state & RADAR_STATE_BLOCKED)
  {
    ++blocked_ticks;
    if (blocked_ticks >= )
  }

#endif
}
