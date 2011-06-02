#include "config.h"

/* test on the table */
/* #define CONFIG_TABLE_TEST 1 */
#define CONFIG_TABLE_TEST 0

#include <stdint.h>
#include "tile.h"
#include "fsm.h"
#include "swatch.h"
#include "aversive.h"
#include "igreboard.h"
#include "swatch.h"
#include "sonar.h"
#include "sharp.h"
#include "lcd.h"


extern igreboard_dev_t igreboard_device;
extern aversive_dev_t aversive_device;


/* globals */

static unsigned int is_red;

#define DONE_REASON_GAMEOVER 0
#define DONE_REASON_SONAR 1
#define DONE_REASON_SHARP 2
#define DONE_REASON_SWITCH 3
#define DONE_REASON_TRAJ 4
#define DONE_REASON_POS 5
static unsigned int done_reason;


/* aversive */

static void wait_abit(void)
{
#if 0
  volatile unsigned int i;
  for (i = 0; i < 1000; ++i) asm("nop");
#else
  const unsigned int msecs = swatch_get_msecs();
  while ((swatch_get_msecs() - msecs) < 100) ;
#endif
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

#if CONFIG_ENABLE_SONAR
  sonar_finalize();
#endif
}


static void first_pos(void)
{
  int16_t posa, posx, posy;
  if (is_red) aversive_set_pos(&aversive_device, 0, 50 + 97, 0);
  else aversive_set_pos(&aversive_device, 180, 3000 - (50 + 97), 0);
  aversive_get_pos(&aversive_device, &posa, &posx, &posy);
  aversive_set_pos(&aversive_device, posa, posx, 2100 - (160 + 50));
}


static void wait_cord(void)
{
#if 0 /* dont use automata, since scheduled */
  fsm_t fsm;
  waitcord_fsm_initialize(&fsm);
  fsm_execute_one(&fsm);
#else
  unsigned int is_pushed;
  unsigned int prev_msecs = swatch_get_msecs();
  unsigned int msecs;
  while (1)
  {
    msecs = swatch_get_msecs();
    if ((msecs - prev_msecs) < 100) continue ;
    prev_msecs = msecs;

    igreboard_get_cord_switch(&igreboard_device, &is_pushed);
    if (is_pushed == 0)
    {
      swatch_start_game();
      break ;
    }
  }
#endif
}


static void goto_first_line(void)
{
#if (CONFIG_TABLE_TEST == 0)
  int16_t a, x, y;
  aversive_get_pos(&aversive_device, &a, &x, &y);
#if 1 /* TABLE */
  if (is_red) x = 780;
  else x = 3000 - 780;
#else
  if (is_red) x = 500;
  else x = 3000 - 500;
#endif
  aversive_goto_xy_abs(&aversive_device, x, y);
  wait_done();
#endif
}


static void turn(void)
{
#if (CONFIG_TABLE_TEST == 0)
  const int16_t a = is_red ? -90 : 90;
  igreboard_open_gripper(&igreboard_device);
  aversive_turn(&aversive_device, a);
  wait_done();
#else
  const unsigned int msecs = swatch_get_msecs();
  igreboard_open_gripper(&igreboard_device);
  while (swatch_get_msecs() - msecs < 1000) ;
#endif
}


static inline unsigned int min(unsigned int a, unsigned int b)
{
  return a < b ? a : b;
}

static void move_until(void)
{
  /* (gameover || pos || sonar || sharp || switch || traj) */

  int is_done;
  unsigned int fl, fr;
  unsigned int is_pushed;
  int16_t a, x, y;

  aversive_move_forward(&aversive_device, 400);

  while (1)
  {
    /* can_poll_bus(igreboard_device.can_dev); */
    wait_abit();

    /* gameover */
    if (swatch_is_game_over())
    {
      done_reason = DONE_REASON_GAMEOVER;
      break ;
    }

    /* position */
    aversive_get_pos(&aversive_device, &a, &x, &y);
    if (y <= 400)
    {
      done_reason = DONE_REASON_POS;
      break ;
    }

#if CONFIG_ENABLE_SONAR
    /* sonar */
    sonar_schedule();
    if (sonar_is_detected())
    {
      done_reason = DONE_REASON_SONAR;
      break ;
    }
#endif /* CONFIG_ENABLE_SONAR */

    /* sharp */
    fl = sharp_read_fl();
    fr = sharp_read_fr();
#define PAWN_DIST 150
    if (min(fl, fr) <= PAWN_DIST)
    {
      done_reason = DONE_REASON_SHARP;
      break ;
    }

    /* switch */
    igreboard_get_gripper_switch(&igreboard_device, &is_pushed);
    if (is_pushed == 1)
    {
      done_reason = DONE_REASON_SWITCH;
      break ;
    }

    /* traj */
    aversive_is_traj_done(&aversive_device, &is_done);
    if (is_done)
    {
      done_reason = DONE_REASON_TRAJ;
      break ;
    }
  }

  if (done_reason != DONE_REASON_TRAJ)
    aversive_stop(&aversive_device);
}

static inline int16_t clamp_x(int16_t x)
{
  if (x < 0) x = 0;
  else if (x > 3000) x = 3000;
  return x;
}

static inline int16_t clamp_y(int16_t y)
{
  if (y < 0) y = 0;
  else if (y > 2100) y = 2100;
  return y;
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

static void do_putpawn_left(void)
{
  const int16_t a = 90;

/*   aversive_move_forward(&aversive_device, 110); */
/*   wait_done(); */

  aversive_turn(&aversive_device, a);
  wait_done();

  aversive_move_forward(&aversive_device, 70);
  wait_done();

  const unsigned int msecs = swatch_get_msecs();
  igreboard_open_gripper(&igreboard_device);
  while (swatch_get_msecs() - msecs < 1000) ;

  aversive_move_forward(&aversive_device, -150);
  wait_done();

  aversive_turn(&aversive_device, -a);
  wait_done();
}

static void do_putpawn_right(void)
{
  const int16_t a = -90;

/*   aversive_move_forward(&aversive_device, 110); */
/*   wait_done(); */

  aversive_turn(&aversive_device, a);
  wait_done();

  aversive_move_forward(&aversive_device, 70);
  wait_done();

  const unsigned int msecs = swatch_get_msecs();
  igreboard_open_gripper(&igreboard_device);
  while (swatch_get_msecs() - msecs < 1000) ;

  aversive_move_forward(&aversive_device, -150);
  wait_done();

  aversive_turn(&aversive_device, -a);
  wait_done();
}

static void center_angular(void)
{
  int16_t a, x, y;
  aversive_get_pos(&aversive_device, &a, &x, &y);

#if 1
  lcd_string(3, 0, "__angle__");
  lcd_string(4, 0, "         ");
  lcd_string(5, 0, "         ");
  lcd_uint16(4, 0, a);
#endif

  if (is_red)
  {
    if ((a <= -90) && (a >= -180)) a = -a - 90;
    else if ((a > -90) && (a <= 0)) a = -(90 + a);
  }
  else
  {
#if 0
    if ((a >= 180) && (a <= 270)) a = 270 - a;
    else if ((a >= 270) && (a < 360)) a = -(a - 270);
#else
    if ((a >= -180) && (a <= -90)) a = -a - 90;
    else if ((a >= -90) && (a < 0)) a = -(90 - a);
#endif
  }

#if 1
  lcd_uint16(5, 0, a);
  while (1) ;
#endif

  aversive_turn(&aversive_device, a);
  wait_done();
}

static void center_tile(void)
{
  unsigned int tilex, tiley;
  int16_t posa, posx, posy;
  int16_t midx, midy;
  int16_t d;

  aversive_get_pos(&aversive_device, &posa, &posx, &posy);

#if 0
  center_angular();
#endif

  tilex = clamp_x(posx);
  tiley = clamp_y(posy);
  world_to_tile(&tilex, &tiley);

#if 0
  lcd_uint16(1, 0, tilex);
  lcd_uint16(2, 0, tiley);
  while (1) ;
#endif

  midy = tiley * 350 + 175;
  if (midy > posy) d = -(midy - posy);
  else d = posy - midy;
  aversive_move_forward(&aversive_device, d);
  wait_done();
}

static void do_putpawn(void)
{
  center_tile();

  if (is_red)
  {
    if (is_left_red()) do_putpawn_left();
    else do_putpawn_right();
  }
  else
  {
    if (is_left_red()) do_putpawn_right();
    else do_putpawn_left();
  }

#if 0
  unsigned int msecs;

  aversive_turn(&aversive_device, 90);
  wait_done();

  aversive_move_forward(&aversive_device, 100);
  wait_done();

  msecs = swatch_get_msecs();
  igreboard_open_gripper(&igreboard_device);
  while ((swatch_get_msecs() - msecs) < 1000) ;

  aversive_move_forward(&aversive_device, -150);
  wait_done();

  aversive_turn(&aversive_device, -90);
  wait_done();

  aversive_move_forward(&aversive_device, 100);
  wait_done();
#endif
}


static unsigned int handle_done(void)
{
  unsigned int can_redo = 0;

  switch (done_reason)
  {
  default:
  case DONE_REASON_POS:
  case DONE_REASON_GAMEOVER:
    {
      aversive_stop(&aversive_device);
      aversive_set_asserv(&aversive_device, 0);
      aversive_set_power(&aversive_device, 0);

#if CONFIG_ENABLE_SONAR
      igreboard_disable_sonar(&igreboard_device);
#endif
      break ;
    }

  case DONE_REASON_SONAR:
    {
      can_redo = 1;
      break ;
    }

  case DONE_REASON_SHARP:
    {
      fsm_t fsm;
      takepawn_fsm_initialize(&fsm);
      fsm_execute_one(&fsm);
      do_putpawn();
      can_redo = 1;
      break ;
    }

  case DONE_REASON_SWITCH:
    {
      const unsigned int msecs = swatch_get_msecs();
      igreboard_close_gripper(&igreboard_device);
      while ((swatch_get_msecs() - msecs) < 1000) ;
      do_putpawn();
      can_redo = 1;
      break ;
    }

  case DONE_REASON_TRAJ:
    {
      can_redo = 1;
      break ;
    }
  }

  return can_redo;
}


/* exported */

void unit_homol(void)
{
  unsigned int can_redo;

  initialize();
  first_pos();
  wait_cord();

#if CONFIG_ENABLE_SONAR
  sonar_initialize();
#endif

  swatch_start_game();
  goto_first_line();
  turn();

 redo_move:
  move_until();
  can_redo = handle_done();
  if (can_redo) goto redo_move;

  while (1) asm("nop");
}
