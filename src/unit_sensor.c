#include "config.h"
#include <stdint.h>
#include "igreboard.h"
#include "swatch.h"
#include "sonar.h"
#include "sharp.h"
#include "lcd.h"

extern igreboard_dev_t igreboard_device;

static void lcd_bitmap(uint8_t row, uint8_t col, uint8_t map)
{
  char buf[] = { 0, 0 };
  unsigned int i;

  for (i = 0; i < 8; ++i, map >>= 1)
  {
    buf[0] = '0' + (char)(map & 1);
    lcd_string(row, col + i * 5, buf);
  }
}

typedef unsigned int (*sharp_fn_t)(void);

void unit_sensor(void)
{
  static const sharp_fn_t sharp_fns[] =
  {
    sharp_read_fr,
    sharp_read_fl,
    sharp_read_fm,
    sharp_read_lh,
    sharp_read_lb,
    sharp_read_lf,
    sharp_read_rh,
    sharp_read_rb,
    sharp_read_rf,
    sharp_read_b
  };

#define SHARP_FN_COUNT (sizeof(sharp_fns) / sizeof(sharp_fns[0]))

  unsigned int msecs[2];
  unsigned int row;
  unsigned int col;
  unsigned int value;
  unsigned int i;
  uint8_t map;

  igreboard_open_gripper(&igreboard_device);
  {
    const unsigned int msecs = swatch_get_msecs();
    while ((swatch_get_msecs() - msecs) < 2000) ;
  }

  msecs[1] = swatch_get_msecs();

  while (1)
  {
#if 0
    /* wait for at least 10ms */
    msecs[0] = swatch_get_msecs();
    if ((msecs[0] - msecs[1]) >= 1000)
      can_poll_bus(igreboard_device.can_dev);
#endif

    /* wait for at least 1 second */
    msecs[0] = swatch_get_msecs();
    if ((msecs[0] - msecs[1]) < 1000) continue ;
    msecs[1] = msecs[0];

    /* 10 sharps, 5 rows x 2 cols */

    row = 0;
    for (i = 0; i < SHARP_FN_COUNT; ++i)
    {
      col = 30;
      if ((i & 1) == 0)
      {
	++row;
	col = 0;
      }

      lcd_uint16(row, col, (uint16_t)sharp_fns[i]());
    }

    /* sonar, 1 row */
#if 0
    if (sonar_is_detected()) lcd_string(row, 0, "1");
    else lcd_string(row, 0, "0");
    ++row;
#endif

    /* switch map, 1 row */
    ++row;
    map = 0;

    igreboard_get_back_switches(&igreboard_device, &value);
    map = value & 3;

    igreboard_get_gripper_switch(&igreboard_device, &value);
    map |= (value & 1) << 2;

    igreboard_get_cord_switch(&igreboard_device, &value);
    map |= (value & 1) << 3;

    igreboard_get_color_switch(&igreboard_device, &value);
    map |= (value & 1) << 4;

    lcd_bitmap(row, 0, map);
  }
}
