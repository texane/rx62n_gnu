#include "config.h"
#include <stdint.h>
#include "igreboard.h"


extern igreboard_dev_t igreboard_device;


/* last read values */
static volatile unsigned int is_detected = 0;


void sonar_initialize(void)
{
  igreboard_enable_sonar(&igreboard_device);
  is_detected = 0;
}


void sonar_finalize(void)
{
  igreboard_disable_sonar(&igreboard_device);
}


unsigned int sonar_is_detected(void)
{
  return is_detected;
}


void sonar_schedule(void)
{
  /* CONFIG_TIMER_FREQ, 200ms */

  static unsigned int pass = 0;

  unsigned int o;
  unsigned int d;

  /* prescaler == 2 */
  if ((++pass) & (2 - 1)) return ;

  if (igreboard_read_sonar(&igreboard_device, &o, &d) == -1)
    return ;

  if (d <= 300) is_detected = 1;
  else is_detected = 0;
}
