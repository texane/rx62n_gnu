#include "config.h"
#include <stdint.h>
#include "igreboard.h"


extern igreboard_dev_t igreboard_device;


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


void radar_read_pos(uint16_t* a, uint16_t* d)
{
  /* a the angle, d the distance of the nearest point */
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
