#include "config.h"
#include <stdint.h>
#include "fsm.h"
#include "swatch.h"
#include "aversive.h"
#include "igreboard.h"
#include "swatch.h"
#include "sonar.h"
#include "lcd.h"


extern igreboard_dev_t igreboard_device;
extern aversive_dev_t aversive_device;


/* globals */

static unsigned int is_red;


/* aversive */

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


static void initialize(void)
{
  /* initialize globals */
  igreboard_get_color_switch(&igreboard_device, &is_red);
}


static void first_pos(void)
{
  fsm_t fsm;
  firstpos_fsm_initialize(&fsm);
  fsm_execute_one(&fsm);
}


static void wait_cord(void)
{
  fsm_t fsm;
  waitcord_fsm_initialize(&fsm);
  fsm_execute_one(&fsm);
}


static void goto_first_line(void)
{
  int16_t a, x, y;
  aversive_get_pos(&aversive_device, &a, &x, &y);
  if (is_red) x = 600;
  else x = 3000 - 600;
  aversive_goto_xy_abs(&aversive_device, x, y);
  wait_done();
}


static void turn(void)
{
#if 0
  const int16_t a = is_red ? ;

  if (is_red) a = 600;
  else x = 3000 - 600;

  wait_done();
#endif
}


static void move_until(void)
{
  /* (sharp || switch || done || gameover || avoid) */
}


/* exported */

void unit_homol(void)
{
  initialize();
  first_pos();
  wait_cord();
  goto_first_line();
  turn();
  move_until();

  while (1) asm("nop");
}
