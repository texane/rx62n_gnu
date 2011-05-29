#include "config.h"
#include <stdint.h>
#include "igreboard.h"


extern igreboard_dev_t igreboard_device;


#define SONAR_STATE_SCAN (1 << 0)
#define SONAR_STATE_BLOCK (1 << 1)
static volatile uint32_t state;

static volatile uint32_t blocked_ticks;


void sonar_initialize(void)
{
#if 0 /* not implemented */
  state = SONAR_STATE_SCAN;
  blocked_count = 0;
#endif
}


void sonar_enable_block(void)
{
}


void sonar_disable_block(void)
{
}

void sonar_enable_scan(void)
{
  state |= SONAR_STATE_SCAN;
}

void sonar_disable_scan(void)
{
  state &= ~SONAR_STATE_SCAN;
}

void sonar_schedule(void)
{
  /* CONFIG_TIMER_FREQ, 100ms */

  static unsigned int pass = 0;

  unsigned int o;
  unsigned int d;

  /* prescaler == 4 */
  if ((++pass) & (4 - 1)) return ;

  if (igreboard_read_sonar(&igreboard_device, &o, &d) == -1)
    return ;

#if 0 /* TODO */
  /* if (d <= 300) ... */
#endif
}
