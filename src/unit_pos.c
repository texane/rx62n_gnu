#include "config.h"
#include <stdint.h>
#include "swatch.h"
#include "aversive.h"
#include "igreboard.h"
#include "lcd.h"


extern igreboard_dev_t igreboard_device;
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
#if 0
    if (can_poll_bus(dev->can_dev) == -1)
    {
      lcd_string(3, 0, "[!] aversive_poll_bus");
      return -1;
    }
#endif
  }

  return 0;
}


void unit_pos(void)
{
  unsigned int is_red;
  int16_t a, x, y;
  
  igreboard_get_color_switch(&igreboard_device, &is_red);

  if (is_red)
  {
    a = 0;
    x = 97;
    y = 2100 - 160;
  }
  else
  {
    a = 180;
    x = 3000 - 97;
    y = 2100 - 160;
  }

/*   aversive_set_pos(&aversive_device, a, x, y); */
/*   aversive_goto_xy_abs(&aversive_device, x + 100, y); */
/*   wait_done(&aversive_device); */

  aversive_set_pos(&aversive_device, a, x, y);
  if (is_red)
    aversive_goto_xy_abs(&aversive_device, x + 100, y - 100);
  else
    aversive_goto_xy_abs(&aversive_device, x - 100, y - 100);
  wait_done(&aversive_device);

  aversive_get_pos(&aversive_device, &a, &x, &y);
  lcd_uint16(1, 0, a);
  lcd_uint16(2, 0, x);
  lcd_uint16(3, 0, y);
}
