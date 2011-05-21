#include "config.h"

#include <stdint.h>
#include "iodefine.h"
#include "yrdkrx62ndef.h"
#include "lcd.h"
#include "sched.h"
#include "hwsetup.h"

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

#elif CONFIG_DO_TAKEPAWN

/* take pawn finite state machine */

typedef struct takepawn_fsm
{
  /* current state */
  enum
  {
    SEARCH = 0,
    CENTER,
    MOVE,
    TAKE,
    WAIT_TRAJ,
    DONE
  } state;

  /* store the previous state */
  unsigned int prev_state;

  /* front left, right sharps */
  unsigned int fl;
  unsigned int fr;

  /* angle */
  int alpha;

} takepawn_fsm_t;

static inline void takepawn_fsm_init(takepawn_fsm_t* fsm)
{
  fsm->state = SEARCH;
  fsm->prev_state = SEARCH;
  fsm->fl = 0;
  fsm->fr = 0;
  fsm->alpha = 0;
}

static inline unsigned int takepawn_fsm_isdone(takepawn_fsm_t* fsm)
{
  return fsm->state == DONE;
}

static inline unsigned int min(unsigned int a, unsigned int b)
{
  return a < b ? a : b;
}

static void takepawn_fsm_next(takepawn_fsm_t* fsm)
{
  switch (fsm->state)
  {
  case SEARCH:
    {
      /* turn looking for the something */
      fsm->fl = sharp_read_fl();
      fsm->fr = sharp_read_fr();

#define PAWN_DIST 200
      if (min(fsm->fl, fsm->fr) < PAWN_DIST)
      {
	fsm->state = CENTER;
	break ;
      }

      /* continue turning */
      aversive_turn(&aversive_device, fsm->alpha);
      fsm->prev_state = SEARCH;
      fsm->state = WAIT_TRAJ;

      break ;
    }

  case WAIT_TRAJ:
    {
      int isdone;
      aversive_is_traj_done(&aversive_device, &isdone);
      if (isdone == 1)
	fsm->state = fsm->prev_state;
      break ;
    }

  case CENTER:
    {
      unsigned int d;

      /* turn until the 2 read approx the same distance */
      fsm->fl = sharp_read_fl();
      fsm->fr = sharp_read_fr();

      if (fsm->fl > fsm->fr) d = fsm->fl - fsm->fr;
      else d = fsm->fr - fsm->fl;

      if (d < 30)
      {
	fsm->state = MOVE;
	break ;
      }

      /* todo: angle should be proportional to d */
      if (fsm->fl > fsm->fr) fsm->alpha = 8;
      else fsm->alpha = -8;

      aversive_turn(&aversive_device, fsm->alpha);
      fsm->prev_state = CENTER;
      fsm->state = WAIT_TRAJ;

      break ;
    }
  
  case MOVE:
    {
      /* assume centered, move forward if not too near */
      
      if (fsm->fl <= 45)
      {
	fsm->state = TAKE;
	break ;
      }

      aversive_move_forward(&aversive_device, 45);

      /* moving may invalidate center */
      fsm->prev_state = CENTER;
      fsm->state = WAIT_TRAJ;

      break ;
    }

  case TAKE:
    {
      fsm->state = DONE;
      break ;
    }

  default:
  case DONE:
    {
      break ;
    }
  }
}

static void do_test(void)
{
  takepawn_fsm_t fsm;

  takepawn_fsm_init(&fsm);
  while (takepawn_fsm_isdone(&fsm) == 0)
    takepawn_fsm_next(&fsm);
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

#if CONFIG_ENABLE_BLINKER
  blinker_schedule();
#endif

#if CONFIG_ENABLE_SWATCH
  swatch_schedule();
#endif

#if CONFIG_ENABLE_RADAR
  radar_schedule();
#endif

#if CONFIG_ENABLE_SHARP
  sharp_schedule();
#endif
}
