#include "config.h"
#include <stdint.h>
#include "lcd.h"
#include "swatch.h"
#include "igreboard.h"


extern igreboard_dev_t igreboard_device;


void unit_igreboard(void)
{
  unsigned int led = 0;
  unsigned int i;
  unsigned int row, col;
  unsigned int values[8];
  unsigned int msecs[2];
  unsigned int openclose = 0;

  msecs[1] = swatch_get_msecs();

  while (1)
  {
    lcd_string(0, 0, "fubaz");

#if CONFIG_ENABLE_IGREBOARD
    can_poll_bus(igreboard_device.can_dev);
#elif (CONFIG_ENABLE_AVERSIVE && CONFIG_USE_CAN)
    can_poll_bus(aversive_device.can_dev);
#endif

    lcd_string(0, 0, "barfu");

    /* wait for at least 1 second */
    msecs[0] = swatch_get_msecs();
    if ((msecs[0] - msecs[1]) < 1000) continue ;
    msecs[1] = msecs[0];

    lcd_string(0, 0, "fubar");

    /* open close gripper */
    if ((openclose & 1) == 0)
      igreboard_open_gripper(&igreboard_device);
    else
      igreboard_close_gripper(&igreboard_device);
    openclose ^= 1;

    /* read gripper switch */
    igreboard_get_gripper_switch(&igreboard_device, &i);
    lcd_uint16(6, 0, (uint16_t)i);

    /* update leds */
    igreboard_set_led(&igreboard_device, 0, led & 1);
    igreboard_set_led(&igreboard_device, 2, led & 1);
    led ^= 1;
    igreboard_set_led(&igreboard_device, 1, led & 1);
    igreboard_set_led(&igreboard_device, 3, led & 1);

    /* display back switches */
    igreboard_get_back_switches(&igreboard_device, &i);
    lcd_uint16(1, 0, (uint16_t)i);

    /* read adcs */
    for (i = 0; i < 8; ++i)
    {
      /* analog input ranges [0:2], [8:12] */
      unsigned int translated = i;
      if (translated > 2) translated += 5;

      if (igreboard_read_adc(&igreboard_device, translated, &values[i]) == -1)
	values[i] = (unsigned int)-1;
    }

    /* display adcs */
    row = 1;
    for (i = 0; i < 8; ++i)
    {
      col = 30;
      if ((i & 1) == 0)
      {
	++row;
	col = 0;
      }

      lcd_uint16((uint8_t)row, (uint8_t)col, (uint16_t)values[i]);
    }

  }
}
