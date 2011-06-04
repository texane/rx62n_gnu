#include "config.h"

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


#define DONE_REASON_GAMEOVER 0
#define DONE_REASON_SONAR 1
#define DONE_REASON_SHARP 2
#define DONE_REASON_SWITCH 3
#define DONE_REASON_TRAJ 4
#define DONE_REASON_POS 5

static unsigned int is_red;


static inline unsigned int can_restart(unsigned int reason)
{
  if ((reason == DONE_REASON_GAMEOVER) || (reason == DONE_REASON_TRAJ))
    return 0;
  return 1;
}


static void wait_abit(void)
{
  const unsigned int msecs = swatch_get_msecs();
  while ((swatch_get_msecs() - msecs) < 100) ;
}

static int wait_done(void)
{
  aversive_dev_t* const dev = &aversive_device;
  int is_done;
  for (is_done = 0; is_done == 0; )
  {
    wait_abit();
    aversive_is_traj_done(dev, &is_done);
  }

  return 0;
}


static void __attribute__((unused)) initialize(void)
{
  /* initialize globals */
  igreboard_get_color_switch(&igreboard_device, &is_red);

#if CONFIG_ENABLE_SONAR
  sonar_initialize();
#endif
}


static void finalize(void)
{
  aversive_stop(&aversive_device);
  aversive_set_asserv(&aversive_device, 0);
  aversive_set_power(&aversive_device, 0);
#if CONFIG_ENABLE_SONAR
  igreboard_disable_sonar(&igreboard_device);
#endif
  igreboard_open_gripper(&igreboard_device);
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

static inline int16_t clamp_a(int16_t a)
{
  int16_t mod = a % 360;
  if (mod < 0) mod = 360 + a;
  return mod;
}

static void orient_north(void)
{
  int16_t a, x, y, beta;
  aversive_get_pos(&aversive_device, &a, &x, &y);

  a = clamp_a(a);

  if (a < 90) beta = 90 - a;
  else if (a < 270) beta = -(a - 90);
  else beta = 90 + 360 - a;

  aversive_turn(&aversive_device, beta);
  wait_done();
}


static void orient_south(void)
{
  int16_t a, x, y, beta;
  aversive_get_pos(&aversive_device, &a, &x, &y);

  a = clamp_a(a);

  if (a > 270) beta = 270 - a;
  else if (a < 90) beta = -(90 + a);
  else beta = 270 - a;

  aversive_turn(&aversive_device, beta);
  wait_done();
}


static void __attribute__((unused)) first_pos(void)
{
  int16_t posa, posx, posy;
  if (is_red) aversive_set_pos(&aversive_device, 0, 50 + 97, 0);
  else aversive_set_pos(&aversive_device, 180, 3000 - (50 + 97), 0);
  aversive_get_pos(&aversive_device, &posa, &posx, &posy);
  aversive_set_pos(&aversive_device, posa, posx, 410);
}


/* helper */

static inline unsigned int min(unsigned int a, unsigned int b)
{
  return a < b ? a : b;
}

static unsigned int wait_something(unsigned int do_pawn, unsigned int is_north)
{
  /* return 1 to restart the move */

  unsigned int fl, fr;
  unsigned int is_pushed;
  int is_done;
  unsigned int reason = DONE_REASON_TRAJ;
  int16_t a, x, y;

  while (1)
  {
    wait_abit();

    /* gameover */
    if (swatch_is_game_over())
    {
      finalize();
      while (1) ;
      break ;
    }

#if CONFIG_ENABLE_SONAR
    /* sonar */
    sonar_schedule();
    if (sonar_is_detected())
    {
      aversive_stop(&aversive_device);
      reason = DONE_REASON_SONAR;
      break ;
    }
#endif /* CONFIG_ENABLE_SONAR */

    /* pawn */
    if (do_pawn)
    {
      /* switch */
      igreboard_get_gripper_switch(&igreboard_device, &is_pushed);
      if (is_pushed == 1)
      {
	aversive_stop(&aversive_device);
	reason = DONE_REASON_SWITCH;
	break ;
      }

      /* front sharps */
      fl = sharp_read_fl();
      fr = sharp_read_fr();
#define PAWN_DIST 170
      if (min(fl, fr) <= PAWN_DIST)
      {
	aversive_stop(&aversive_device);
	reason = DONE_REASON_SHARP;
	break ;
      }

      /* lateral sharp */
#define PAWN_LAT_DIST 300
      fl = is_red ? sharp_read_rb() : sharp_read_lb();
      if (fl < PAWN_LAT_DIST)
      {
	const int16_t a = is_red ? -90 : 90;
	aversive_stop(&aversive_device);
	aversive_turn(&aversive_device, a);
	wait_done();
	reason = DONE_REASON_SHARP;
	break ;
      }
    }

    /* position */
    aversive_get_pos(&aversive_device, &a, &x, &y);
    if (is_north && (y >= 1600))
    {
      aversive_stop(&aversive_device);
      reason = DONE_REASON_POS;
      break ;
    }
    else if (y <= 350)
    {
      aversive_stop(&aversive_device);
      reason = DONE_REASON_POS;
      break ;
    }

    /* traj */
    aversive_is_traj_done(&aversive_device, &is_done);
    if (is_done)
    {
      reason = DONE_REASON_TRAJ;
      break ;
    }
  }

  return reason;
}

/* sub */

static int move_until_pawn(void)
{
  /* return 0 if pawn, -1  otherwise */
  /* assume starting at middle of second case, south */

  unsigned int reason;

 restart_move:
  aversive_move_forward(&aversive_device, 2000);
  reason = wait_something(1, 1);

  if ((reason == DONE_REASON_SHARP) || (reason = DONE_REASON_SWITCH))
    return 0;

  if (can_restart(reason)) goto restart_move;
  return -1;
}


static void move_to_bonus(void)
{
  unsigned int reason;
  int16_t prevy;
  int16_t a, x, y;
  int is_done;

  x = 450 + 350 * 2 + 350;
  if (is_red == 0) x = 3000 - x;
  y = 350 / 2;

 restart_move:
  aversive_goto_forward_xy_abs(&aversive_device, x, y);
  reason = wait_something(0, 0);
  if (can_restart(reason)) goto restart_move;

  /* move a bit */
  if (reason == DONE_REASON_TRAJ)
  {
    aversive_get_pos(&aversive_device, &a, &x, &prevy);
    aversive_move_forward(&aversive_device, 25);

    /* wait until done or not moving anymore */
    while (1)
    {
      swatch_wait_msecs(200);
      aversive_is_traj_done(&aversive_device, &is_done);
      if (is_done) break ;
      aversive_get_pos(&aversive_device, &a, &x, &y);
      if (prevy == y)
      {
	aversive_stop(&aversive_device);
	break ;
      }
      prevy = y;
    }
  }
}

static void center_table(void)
{
  int16_t a, x, y;
  aversive_get_pos(&aversive_device, &a, &x, &y);
  aversive_goto_forward_xy_abs(&aversive_device, 3000 / 2, y);
  wait_something(0, 0);
}

static void take_pawn(void)
{
  fsm_t fsm;
  unsigned int is_pushed;

  igreboard_get_gripper_switch(&igreboard_device, &is_pushed);
  if (is_pushed == 1)
  {
    igreboard_close_gripper(&igreboard_device);
    return ;
  }

  /* do twice, may be pushed after moving forward a bit */
  aversive_move_forward(&aversive_device, 15);
  wait_done();
  igreboard_get_gripper_switch(&igreboard_device, &is_pushed);
  if (is_pushed == 1)
  {
    igreboard_close_gripper(&igreboard_device);
    return ;
  }

  takepawn_fsm_initialize(&fsm);
  fsm_execute_one(&fsm);
}

extern unsigned int last_tiley;

void unit_bonus(void)
{
#if 0
  initialize();
  first_pos();
#else
  igreboard_get_color_switch(&igreboard_device, &is_red);

  if (last_tiley <= 1)
  {
    /* filled by the last put pawn */
#if 0
    aversive_move_forward(&aversive_device, -380);
    wait_done();
    if (is_red) aversive_turn(&aversive_device, 90);
    else aversive_turn(&aversive_device, -90);
    wait_done();
    aversive_move_forward(&aversive_device, 350);
    wait_done();
#else
    int16_t a, x, y;

    aversive_get_pos(&aversive_device, &a, &x, &y);
    y = 350 * 2 + 350 / 2;
    aversive_goto_xy_abs(&aversive_device, x, y);
    wait_done();

    aversive_get_pos(&aversive_device, &a, &x, &y);
    x = 450 + 350 * 2 + 350 / 2;
    if (is_red == 0) x = 3000 - x;
    aversive_goto_xy_abs(&aversive_device, x, y);
    wait_done();
#endif
  }
  else /* last cell not filled */
  {
    if (is_red) aversive_turn(&aversive_device, 95);
    else aversive_turn(&aversive_device, -95);
    wait_done();
#if 0
    aversive_move_forward(&aversive_device, 510);
#else
    int16_t a, x, y;
    aversive_get_pos(&aversive_device, &a, &x, &y);
    x = 450 + 350 * 2 + 350 / 2;
    if (is_red == 0) x = 3000 - x;
    aversive_goto_forward_xy_abs(&aversive_device, x, y + 100);
#endif
    wait_done();
  }

  orient_north();

#endif

  if (move_until_pawn() == 0)
  {
    take_pawn();
    center_table();
    orient_south();
    move_to_bonus();
    igreboard_open_gripper(&igreboard_device);
  }

  finalize();

  while (1) ;
}
