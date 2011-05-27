#include "config.h"
#include <stdint.h>
#include "lcd.h"
#include "aversive.h"


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

static int do_turn(aversive_dev_t* dev, unsigned int a)
{
  /* a the angle in degrees */

  if (aversive_turn(dev, a) == -1)
  {
    lcd_string(3, 0, "[!] aversive_turn");
    return -1;
  }

  return wait_done(dev);
}

static int do_move(aversive_dev_t* dev, unsigned int d)
{
  /* d the distance in mm */

  if (aversive_move_forward(dev, d) == -1)
  {
    lcd_string(3, 0, "[!] aversive_turn");
    return -1;
  }

  return wait_done(dev);
}

static int do_square(aversive_dev_t* dev)
{
  unsigned int i;

  for (i = 0; i < 4; ++i)
  {
    if (do_move(dev, 1000) == -1) return -1;
    if (do_turn(dev, 90) == -1) return -1;
  }

  return 0;
}

void unit_square(void)
{
  lcd_string(2, 0, "do_test   ");

  while (1)
  {
    wait_abit();
    do_square(&aversive_device);
  }
}
