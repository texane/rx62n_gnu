#include "config.h"
#include <stdint.h>
#include "swatch.h"
#include "lcd.h"
#include "igreboard.h"


extern igreboard_dev_t igreboard_device;


static inline char nibble_to_ascii(uint8_t value)
{
  if (value >= 0xa) return 'a' + value - 0xa;
  return '0' + value;
}

static const char* uint16_to_string(uint16_t value)
{
  static char buf[8];
  unsigned int i;

  for (i = 0; i < 4; ++i, value >>= 4)
    buf[4 - i - 1] = nibble_to_ascii(value & 0xf);
  buf[i] = 0;

  return buf;
}

static inline void print_uint16
(unsigned int row, unsigned int col, uint16_t value)
{
  lcd_string((uint8_t)row, (uint8_t)col, uint16_to_string(value));
}

void unit_sonar(void)
{
  unsigned int msecs[2];
  unsigned int o;
  unsigned int d;

  msecs[1] = swatch_get_elapsed_msecs();

  igreboard_enable_sonar(&igreboard_device);

  while (1)
  {
    can_poll_bus(igreboard_device.can_dev);

    /* wait for at least 1 second */
    msecs[0] = swatch_get_elapsed_msecs();
    if ((msecs[0] - msecs[1]) < 1000) continue ;
    msecs[1] = msecs[0];

    igreboard_read_sonar(&igreboard_device, &o, &d);

    print_uint16(4, 0, (uint16_t)o);
    print_uint16(4, 30, (uint16_t)d);
  }

  igreboard_disable_sonar(&igreboard_device);
}
