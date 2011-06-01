#include "config.h"
#include <stdint.h>
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
  const int16_t a = is_red ? -90 : 90;
  aversive_turn(&aversive_device, a);
  wait_done();
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
    wait_abit();

    /* gameover */
    if (swatch_is_game_over())
    {
      done_reason = DONE_REASON_GAMEOVER;
      break ;
    }

    /* position */
    aversive_get_pos(&aversive_device, &a, &x, &y);
    if (y <= 200)
    {
      done_reason = DONE_REASON_POS;
      break ;
    }

    /* sonar */
    if (sonar_is_detected())
    {
      done_reason = DONE_REASON_SONAR;
      return ;
    }

    /* sharp */
    fl = sharp_read_fl();
    fr = sharp_read_fr();
#define PAWN_DIST 200
    if (min(fl, fr) <= PAWN_DIST)
    {
      done_reason = DONE_REASON_SHARP;
      return ;
    }

    /* switch */
    igreboard_get_gripper_switch(&igreboard_device, &is_pushed);
    if (is_pushed == 1)
    {
      done_reason = DONE_REASON_SWITCH;
      return ;
    }

    /* traj */
    aversive_is_traj_done(&aversive_device, &is_done);
    if (is_done)
    {
      done_reason = DONE_REASON_TRAJ;
      return ;
    }
  }

  if (done_reason != DONE_REASON_TRAJ)
    aversive_stop(&aversive_device);
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
      break ;
    }

  case DONE_REASON_SONAR:
    {
      can_redo = 1;
      break ;
    }

  case DONE_REASON_SHARP:
  case_done_reason_sharp:
    {
      can_redo = 1;
      break ;
    }

  case DONE_REASON_SWITCH:
    {
      aversive_move_forward(&aversive_device, -100);
      goto case_done_reason_sharp;
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
  swatch_start_game();
  goto_first_line();
  turn();

 redo_move:
  move_until();
  can_redo = handle_done();
  if (can_redo) goto redo_move;

  while (1) asm("nop");
}
