#include "iodefine.h"
#include "yrdkrx62ndef.h"
#include "lcd.h"
#include "clock.h"
#include "aversive.h"
#include "hwsetup.h"
#include "swatch_task.h"
#include "led_task.h"


/* can globals */
extern can_std_frame_t tx_dataframe;
extern can_std_frame_t rx_dataframe;
extern can_std_frame_t remote_frame;

#ifndef USE_CAN_POLL
extern uint32_t can0_tx_sentdata_flag;
extern uint32_t can0_tx_remote_sentdata_flag;
extern uint32_t can0_rx_newdata_flag;
extern uint32_t can0_rx_test_newdata_flag;
extern uint32_t can0_rx_remote_frame_flag;
#endif /* !USE_CAN_POLL */


/* global aversive device context */
aversive_dev_t aversive_device;


/* demo program */
static void wait_abit(void)
{
  volatile unsigned int i;
  for (i = 0; i < 2000; ++i) asm("wait");
}

static int wait_done(aversive_dev_t* dev)
{
  int is_done;

  for (is_done = 0; is_done == 0; )
  {
    wait_abit();
    aversive_is_traj_done(dev, &is_done);

    if (aversive_poll_bus(dev) == -1)
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
    if (do_move(dev, 500) == -1) return -1;
    if (do_turn(dev, 90) == -1) return -1;
  }

  return 0;
}

static void do_test(aversive_dev_t* dev)
{
  lcd_string(2, 0, "do_test   ");

#if 0 /* disable aversive */
  while (1)
  {
    wait_abit();
    do_square(dev);
  }
#endif

}


/* global initialization routines */

static int initialize(void)
{
  /* 100MHz CPU clock, configure ports, peripherals ... */
  HardwareSetup();

  /* init devices */
  lcd_open();
  lcd_string(2, 0, "initialize");

#if 0 /* disable_aversive */

  if (aversive_open(&aversive_device) == -1)
    return -1;

#endif

  /* initialize the tasks */
  led_task_initialize();
  swatch_task_initialize();

  /* start the scheduler */
  tick_start();

  return 0;
}

static void finalize(void)
{
  aversive_close(&aversive_device);
}


/* called on each timer interrupt */

void tick_isr(void) 
{
  led_task_schedule();
  swatch_task_schedule();
}


/* main */

int main(void)
{
  initialize();
  do_test(&aversive_device);
  finalize();

  return 0;
}
