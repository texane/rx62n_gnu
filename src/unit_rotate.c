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

static int wait_done(void)
{
  aversive_dev_t* const dev = &aversive_device;
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


void unit_rotate(void)
{
  int16_t a, x, y;

  aversive_turn(&aversive_device, 360);
  wait_done();

  aversive_turn(&aversive_device, 360);
  wait_done();

  aversive_turn(&aversive_device, 720);
  wait_done();

  aversive_get_pos(&aversive_device, &a, &x, &y);

  lcd_uint16(1, 0, a);
  lcd_uint16(2, 0, a);
  lcd_uint16(3, 0, a);
}
