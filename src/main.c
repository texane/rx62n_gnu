#include "config.h"

#include <stdint.h>
#include "iodefine.h"
#include "yrdkrx62ndef.h"
#include "lcd.h"
#include "sched.h"
#include "hwsetup.h"
#include "fsm.h"

#if CONFIG_ENABLE_GRIPPER
# include "gripper.h"
#endif

#if CONFIG_ENABLE_SWITCHES
# include "switches.h"
#endif

#if CONFIG_ENABLE_SWATCH
# include "swatch.h"
#endif

#if CONFIG_ENABLE_BLINKER
# include "blinker.h"
#endif

#if CONFIG_ENABLE_RADAR
# include "radar.h"
#endif

#if CONFIG_ENABLE_SHARP
# include "sharp.h"
#endif

#if CONFIG_ENABLE_IGREBOARD
# include "igreboard.h"
#endif

#if CONFIG_ENABLE_AVERSIVE

# include "aversive.h"

/* global aversive device context */
aversive_dev_t aversive_device;

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

#endif /* CONFIG_ENABLE_AVERSIVE */


/* test programs */

#if CONFIG_DO_KEYVAL

static inline char nibble_to_ascii(uint8_t value)
{
  if (value >= 0xa) return 'a' + value - 0xa;
  return '0' + value;
}

static const char* uint16_to_string(uint16_t value)
{
  static char buf[8];
  unsigned int i;

  for (i = 0; i < 4; ++i, value >>= 4)
    buf[4 - i - 1] = nibble_to_ascii(value & 0xf);
  buf[i] = 0;

  return buf;
}

static void do_test(void)
{
  aversive_dev_t* const dev = &aversive_device;

  uint16_t pass = 0;

#define MAX_KEYS 0x10
  uint16_t expected_vals[MAX_KEYS];

  uint16_t key, val;

  for (key = 0; key < MAX_KEYS; ++key)
  {
    if (aversive_write_keyval(dev, key, key) == -1)
    {
      lcd_string(3, 0, "[!] 0");
      return ;
    }

    expected_vals[key] = key;
  }

  while (1)
  {
    aversive_poll_bus(dev);

    for (key = 0; key < MAX_KEYS; ++key)
    {
      if (aversive_read_keyval(dev, key, &val) == -1)
      {
	lcd_string(3, 0, "[!] R");
	return ;
      }

      if (expected_vals[key] != val)
      {
	lcd_string(3, 0, "[!] R");
	return ;
      }

      if (aversive_write_keyval(dev, key, val + 1) == -1)
      {
	lcd_string(3, 0, "[!] W");
	return ;
      }

      expected_vals[key] = (val + 1) & 0xffff;

#if 0
      /* report frame rate once every 0x20 wrapping */
      if ((key == 0) && ((val & 0x1f) == 0))
      {
	lcd_string(3, 10, uint16_to_string(val));
      }
#endif

    }

    lcd_string(3, 10, uint16_to_string(pass++));
  }
}

#elif CONFIG_DO_SQUARE

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
    if (do_move(dev, 1000) == -1) return -1;
    if (do_turn(dev, 90) == -1) return -1;
  }

  return 0;
}

static void do_test(void)
{
  lcd_string(2, 0, "do_test   ");

  while (1)
  {
    wait_abit();
    do_square(&aversive_device);
  }
}

#elif CONFIG_DO_TAKEPAWN

static void do_test(void)
{
  fsm_t fsm;

  takepawn_fsm_initialize(&fsm);

  while (fsm.is_done(fsm.data) == 0)
  {
    if (swatch_is_game_over())
    {
      fsm.preempt(fsm.data);
      break ;
    }

    fsm.next(fsm.data);
  }

  /* game is over */
#if CONFIG_ENABLE_AVERSIVE
  aversive_stop(&aversive_device);
  aversive_set_asserv(&aversive_device, 0);
  aversive_set_power(&aversive_device, 0);
#endif
}

#elif CONFIG_DO_IGREBOARD

static inline char nibble_to_ascii(uint8_t value)
{
  if (value >= 0xa) return 'a' + value - 0xa;
  return '0' + value;
}

static const char* uint16_to_string(uint16_t value)
{
  static char buf[8];
  unsigned int i;

  for (i = 0; i < 4; ++i, value >>= 4)
    buf[4 - i - 1] = nibble_to_ascii(value & 0xf);
  buf[i] = 0;

  return buf;
}

static inline void print_uint16
(unsigned int row, unsigned int col, uint16_t value)
{
  lcd_string((uint8_t)row, (uint8_t)col, uint16_to_string(value));
}

static void do_test(void)
{
  unsigned int led = 0;
  unsigned int i;
  unsigned int row, col;
  unsigned int values[8];
  unsigned int msecs[2];
  unsigned int openclose = 0;

  msecs[1] = swatch_get_elapsed_msecs();

  while (1)
  {
    lcd_string(0, 0, "fubaz");

    aversive_poll_bus(&aversive_device);

    lcd_string(0, 0, "barfu");

    /* wait for at least 1 second */
    msecs[0] = swatch_get_elapsed_msecs();
    if ((msecs[0] - msecs[1]) < 1000) continue ;
    msecs[1] = msecs[0];

    lcd_string(0, 0, "fubar");

    /* open close gripper */
    if ((openclose & 1) == 0)
      igreboard_open_gripper();
    else
      igreboard_close_gripper();
    openclose ^= 1;

    /* update leds */
    igreboard_set_led(0, led & 1);
    igreboard_set_led(2, led & 1);
    led ^= 1;
    igreboard_set_led(1, led & 1);
    igreboard_set_led(3, led & 1);

    /* read adcs */
    for (i = 0; i < 8; ++i)
    {
      /* analog input ranges [0:2], [8:12] */
      unsigned int translated = i;
      if (translated > 2) translated += 5;

      if (igreboard_read_adc(translated, &values[i]) == -1)
	values[i] = (unsigned int)-1;
    }

    /* display adcs */
    row = 0;
    for (i = 0; i < 8; ++i)
    {
      col = 30;
      if ((i & 1) == 0)
      {
	++row;
	col = 0;
      }

      print_uint16(row, col, (uint16_t)values[i]);
    }

  }
}

#else

static void do_test(void)
{
  while (1) asm("wait\n");
}

#endif /* CONFIG_DO_SQUARE */


/* global initialization routines */

static int initialize(void)
{
  /* 100MHz CPU clock, configure ports, peripherals ... */
  HardwareSetup();

  /* init devices */
  lcd_open();

#if CONFIG_ENABLE_AVERSIVE
  if (aversive_open(&aversive_device) == -1)
  {
    lcd_string(3, 0, "[!] aversive_open");
    return -1;
  }
#endif /* CONFIG_ENABLE_AVERSIVE */

#if CONFIG_ENABLE_BLINKER
  blinker_initialize();
#endif

#if CONFIG_ENABLE_SHARP
  sharp_initialize_all();
#endif

#if CONFIG_ENABLE_SWITCHES
  switches_initialize();
#endif

#if CONFIG_ENABLE_SWATCH
  swatch_initialize();
#endif

#if CONFIG_ENABLE_RADAR
  radar_initialize();
#endif

#if CONFIG_ENABLE_GRIPPER
  gripper_initialize();
#endif

  /* init and start the scheduler */
  sched_initialize();

  return 0;
}

static void finalize(void)
{
#if CONFIG_ENABLE_AVERSIVE
  aversive_close(&aversive_device);
#endif
}


/* main */

int main(void)
{
  if (initialize() == -1)
  {
    while (1) asm("wait");
  }

  do_test();

  finalize();

  return 0;
}


/* dont move interrupt handlers */

void tick_isr(void) 
{
#if CONFIG_ENABLE_SWITCHES
  switches_schedule();
#endif

#if CONFIG_ENABLE_BLINKER
  blinker_schedule();
#endif

#if CONFIG_ENABLE_SWATCH
  swatch_schedule();
#endif

#if CONFIG_ENABLE_RADAR
  radar_schedule();
#endif
}
