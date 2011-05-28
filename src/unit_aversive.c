#include "config.h"
#include <stdint.h>
#include "aversive.h"
#include "lcd.h"

extern aversive_dev_t aversive_device;

static void wait_abit(void)
{
  volatile unsigned int i;
  for (i = 0; i < 1000; ++i) asm("nop");
}

static int wait_done(aversive_dev_t* dev)
{
  int is_done;

  for (is_done = 0; is_done == 0; )
  {
    wait_abit();
    aversive_is_traj_done(dev, &is_done);

    if (can_poll_bus(dev->can_dev) == -1)
    {
      lcd_string(3, 0, "[!] aversive_poll_bus");
      return -1;
    }
  }

  return 0;
}

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

void unit_aversive(void)
{
  int16_t a, x, y;

  aversive_goto_xy_abs(&aversive_device, 100, 100);
  wait_done(&aversive_device);
  aversive_get_pos(&aversive_device, &a, &x, &y);
  lcd_string(3, 0, uint16_to_string((uint16_t)x));
  lcd_string(3, 30, uint16_to_string((uint16_t)y));
}
