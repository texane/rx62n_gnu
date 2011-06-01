#include "config.h"
#include <stdint.h>
#include "igreboard.h"


extern igreboard_dev_t igreboard_device;


/* last read values */
static unsigned int detect_count = 0;
static volatile unsigned int is_detected = 0;


void sonar_initialize(void)
{
  igreboard_enable_sonar(&igreboard_device);
  is_detected = 0;
  detect_count = 0;
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

/*   static unsigned int pass = 0; */

  unsigned int o;
  unsigned int d;

  /* prescaler == 2 */
/*   if ((++pass) & (2 - 1)) return ; */
/*   if ((++pass) & (4096 - 1)) return ; */

  if (igreboard_read_sonar(&igreboard_device, &o, &d) == -1)
  {
    detect_count = 0;
    is_detected = 0;
    return ;
  }

  /* by experiment, 37 a good guess */
  if (d <= 37)
  {
    detect_count = 5;
    is_detected = 1;
  }
  else
  {
    /* wait a bit before assuming undetected */
    if (detect_count == 0)
      is_detected = 0;
    else
      --detect_count;
  }
}
