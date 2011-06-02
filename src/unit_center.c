#include "config.h"
#include <stdint.h>
#include "swatch.h"
#include "aversive.h"
#include "igreboard.h"
#include "lcd.h"


extern igreboard_dev_t igreboard_device;
extern aversive_dev_t aversive_device;


static unsigned int is_red;


static void wait_abit(void)
{
  volatile unsigned int i;
  for (i = 0; i < 1000; ++i) asm("nop");
}

static int wait_done(void)
{
  aversive_dev_t* const dev = &aversive_device;

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


static void first_pos(void)
{
  int16_t posa, posx, posy;
  if (is_red) aversive_set_pos(&aversive_device, 0, 50 + 97, 0);
  else aversive_set_pos(&aversive_device, 180, 3000 - (50 + 97), 0);
  aversive_get_pos(&aversive_device, &posa, &posx, &posy);
  aversive_set_pos(&aversive_device, posa, posx, 2100 - (160 + 50));
}

static void goto_first_line(void)
{
  int16_t a, x, y;
  aversive_get_pos(&aversive_device, &a, &x, &y);
  if (is_red) x = 500;
  else x = 3000 - 500;
  aversive_goto_xy_abs(&aversive_device, x, y);
  wait_done();
}

void unit_center(void)
{
  igreboard_get_color_switch(&igreboard_device, &is_red);

  first_pos();
  goto_first_line();

  aversive_goto_xy_abs(&aversive_device, 3000 / 2, 2100 / 2);
  wait_done();
}
