#include "config.h"

#include <stdint.h>
#include "iodefine.h"
#include "yrdkrx62ndef.h"
#include "lcd.h"
#include "sched.h"
#include "hwsetup.h"
#include "switches.h"
#include "swatch.h"
#include "blinker.h"
#include "radar.h"
#include "adc.h"

#if CONFIG_ENABLE_AVERSIVE

# include "aversive.h"

/* global aversive device context */
static aversive_dev_t aversive_device;

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

      /* report frame rate once every 0x20 wrapping */
      if ((key == 0) && ((val & 0x1f) == 0))
      {
	lcd_string(3, 10, uint16_to_string(val));
      }

    }
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

#elif CONFIG_DO_PAWN

/* take pawn finite state machine */

typedef struct takepawn_fsm
{
  /* current state */
  enum
  {
    INIT = 0,
    CENTER,
    MOVE,
    TAKE,
    DONE
  } state;

  /* front left, right sharps */
  uint16_t fl;
  uint16_t fr;

  /* angle */
  int alpha;

} takepawn_fsm_t;

static inline void init_takepawn_fsm(takepawn_fsm_t* fsm)
{
  fsm->state = INIT;
}

static void schedule_takepawn_fsm(takepawn_fsm_t* fsm)
{
  switch (fsm->state)
  {
  default:
  case INIT:
    {
      break ;
    }

  case CENTER:
    {
      break ;
    }
  
  case MOVE:
    {
      break ;
    }

  case TAKE:
    {
      break ;
    }

  case DONE:
    {
      break ;
    }
  }
}

static inline int is_takepawn_fsm_done(takepawn_fsm_t* fsm)
{
  return fsm->state == DONE;
}

static void do_test(void)
{
  /* turn until pawn detected */

#define FR_SHARP_INDEX 0
#define FL_SHARP_INDEX 1

#define MAX_PAWN_DIST 200

  uint16_t fl, fr;

  while (1)
  {
    fl = adc_read(FL_SHARP_INDEX);
    fr = adc_read(FR_SHARP_INDEX);

    if ((fl < MAX_PAWN_DIST) || (fr < MAX_PAWN_DIST))
      break ;
  }

  /* found a sharp */



  unsigned int alpha;
  uint16_t fl, fr;
  unsigned int ispawn = 0;

  while (1)
  {
    switch (state)
    {
    case TURN_LEFT:
      turn_left(alpha);
      fr = sharp_read_fl();
      if (fr > dist)
      break ;

    case TURN_RIGHT:
      break ;

    case DEFAULT:
    default:
      fl = sharp_read_fl();
      fr = sharp_read_fr();

      if (fl <= DIST_MAX)
      {
	alpha = 10;
	state = TURN_LEFT;
      }
      else if (fr <= DIST_MAX)
      {
      }

      break ;
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
  lcd_string(2, 0, "initialize");

#if CONFIG_ENABLE_AVERSIVE
  if (aversive_open(&aversive_device) == -1)
  {
    lcd_string(3, 0, "[!] aversive_open");
    return -1;
  }
  lcd_string(2, 0, "aversived ");
#endif /* CONFIG_ENABLE_AVERSIVE */

  blinker_initialize();

#if CONFIG_ENABLE_ADC
  adc_initialize();
  lcd_string(2, 0, "adced     ");
#endif

  /* initialize the tasks */

#if CONFIG_ENABLE_SWITCHES
  switches_initialize();
#endif

  swatch_initialize();
  radar_initialize();

  lcd_string(2, 0, "tasked    ");

  /* init and start the scheduler */
  sched_initialize();

  lcd_string(2, 0, "scheduled ");

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

  blinker_schedule();
  swatch_schedule();
  radar_schedule();

#if CONFIG_ENABLE_ADC
  adc_schedule();
#endif
}
