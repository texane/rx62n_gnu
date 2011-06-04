#include "config.h"
#include <stdint.h>
#include "swatch.h"
#include "aversive.h"
#include "igreboard.h"
#include "lcd.h"


extern igreboard_dev_t igreboard_device;
extern aversive_dev_t aversive_device;


static unsigned int is_red;

static inline int16_t clamp_y(int16_t y)
{
  if (y < 0) y = 0;
  else if (y > 2100) y = 2100;
  return y;
}


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


static inline unsigned int is_left_red(void)
{
  /* does not depend on the color */
  int16_t a, x, y;
  aversive_get_pos(&aversive_device, &a, &x, &y);
  y = 2100 - clamp_y(y);
  /* even left reds */
  return ((y / 350) & 1) == 0;
}

static void turn(void)
{
  const int16_t a = is_red ? -90 : 90;
  igreboard_open_gripper(&igreboard_device);
  aversive_turn(&aversive_device, a);
  wait_done();
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

/* static inline int16_t clamp_y(int16_t y) */
/* { */
/*   if (y < 0) y = 0; */
/*   else if (y > 2100) y = 2100; */
/*   return y; */
/* } */

static void center_tile(void)
{
#if 0
  int16_t posa, posx, posy;
  int16_t d;

  aversive_get_pos(&aversive_device, &posa, &posx, &posy);
  d = clamp_y(2100 - posy) % 350;
  if (d < 300)
  {
    aversive_move_forward(&aversive_device, d);
    wait_done();
  }
#else
  aversive_move_forward(&aversive_device, 175);
  wait_done();
#endif
}

void unit_pos(void)
{
  /* int16_t posa, posx, posy; */
  int is_done;
  unsigned int prev_is_red = (unsigned int)-1;
  
  igreboard_get_color_switch(&igreboard_device, &is_red);

  first_pos();
  goto_first_line();
  turn();

  /* goto first line */

 redo_move:
  aversive_move_forward(&aversive_device, 1000);

  for (is_done = 0; is_done == 0; )
  {
    wait_abit();

    if (prev_is_red == (unsigned int)-1)
    {
      prev_is_red = is_left_red();
    }
    else if (is_left_red() != prev_is_red)
    {
      unsigned int msecs = swatch_get_msecs();
      aversive_stop(&aversive_device);
      prev_is_red = is_left_red();
      while ((swatch_get_msecs() - msecs) < 1000);

      if (is_left_red() == is_red)
      {
	center_tile();
/* 	aversive_move_forward(&aversive_device, 175); */
/* 	wait_done(); */

	aversive_turn(&aversive_device, 90);
	wait_done();

	aversive_turn(&aversive_device, -90);
	wait_done();
      }

      goto redo_move;
    }

    if (is_left_red()) lcd_string(1, 0, "red ");
    else lcd_string(1, 0, "blue");
    aversive_is_traj_done(&aversive_device, &is_done);
  }
}
