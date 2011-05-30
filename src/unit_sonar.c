#include "config.h"
#include <stdint.h>
#include "swatch.h"
#include "lcd.h"
#include "igreboard.h"


extern igreboard_dev_t igreboard_device;


void unit_sonar(void)
{
  unsigned int msecs[2];
  unsigned int o;
  unsigned int d;

  msecs[1] = swatch_get_msecs();

  igreboard_enable_sonar(&igreboard_device);

  while (1)
  {
    can_poll_bus(igreboard_device.can_dev);

    /* wait for at least 1 second */
    msecs[0] = swatch_get_msecs();
    if ((msecs[0] - msecs[1]) < 1000) continue ;
    msecs[1] = msecs[0];

    igreboard_read_sonar(&igreboard_device, &o, &d);

    lcd_uint16(4, 0, (uint16_t)o);
    lcd_uint16(4, 30, (uint16_t)d);
  }

  igreboard_disable_sonar(&igreboard_device);
}
