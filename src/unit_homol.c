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


typedef enum
{
  INIT = 0,
  WAIT_CORD,
  WAIT_CORD,
  AVOID,
  WAIT_TURN,
  WAIT_TRAJ,
  TRAJ_DONE,
  INVALID
} state_t;

static state_t state;


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


static unsigned int is_red;

static void initialize(void)
{
  igreboard
}

static void goto_first_line(void)
{
}

static void


void unit_homol(void)
{
  /* algorithm:
     wait_cord();
     recal();
     goto_first_line();
     turn_90();
     move_forward_until(sharp || switch || done || gameover || avoid)
     {

     }

     find nearest tile
     putpawn()
   */

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
