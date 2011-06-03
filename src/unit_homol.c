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
#include "unit.h"


extern igreboard_dev_t igreboard_device;
extern aversive_dev_t aversive_device;


/* globals */

static unsigned int is_red;
static unsigned int direction;

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

static inline unsigned int min(unsigned int a, unsigned int b)
{
  return a < b ? a : b;
}

static int wait_done_or_detected(int16_t x, int16_t y)
{
  aversive_dev_t* const dev = &aversive_device;
  unsigned int fl, fr;
  int is_done;

 restart_goto:
  for (is_done = 0; is_done == 0; )
  {
    wait_abit();

    /* gameover */
    if (swatch_is_game_over())
    {
    game_is_over_label:
      done_reason = DONE_REASON_GAMEOVER;
      return -1;
    }

#if CONFIG_ENABLE_SONAR
    /* sonar */
    sonar_schedule();
    if (sonar_is_detected())
    {
      aversive_stop(&aversive_device);
    do_sonar:
      while (sonar_is_detected())
      {
	swatch_wait_msecs(100);
	if (swatch_is_game_over()) goto game_is_over_label;
      }
      aversive_goto_forward_xy_abs(&aversive_device, x, y);
      goto restart_goto;
    }
#endif /* CONFIG_ENABLE_SONAR */

    /* sharps */
    fl = sharp_read_fl();
    fr = sharp_read_fr();
#define PAWN_DIST 170
    if (min(fl, fr) <= PAWN_DIST)
    {
      aversive_stop(&aversive_device);

#if CONFIG_ENABLE_SONAR
      sonar_schedule();
      if (sonar_is_detected()) goto do_sonar;
#endif /* CONFIG_ENABLE_SONAR */

      aversive_turn(&aversive_device, is_red ? 90 : -90);
      wait_done();
      aversive_move_forward(&aversive_device, 300);
      wait_done();
      aversive_turn(&aversive_device, is_red ? -90 : 90);
      wait_done();

      goto restart_goto;
    }

    aversive_is_traj_done(dev, &is_done);
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

  /* descending */
  direction = 0;
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
  while (1)
  {
    swatch_wait_msecs(100);
    igreboard_get_cord_switch(&igreboard_device, &is_pushed);
    if (is_pushed == 0)
    {
      swatch_start_game();
      break ;
    }
  }
#endif
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

static void do_putpawn(int16_t);

static void do_first_pawn(void)
{
  const int16_t a = is_red ? 6 : -7;

  fsm_t fsm;

  takepawn_fsm_initialize(&fsm);
  fsm_execute_one(&fsm);
  orient_south();
  aversive_move_forward(&aversive_device, -50);
  wait_done();
  do_putpawn(is_red ? 70 : 60);

  /* move a bit */
  aversive_turn(&aversive_device, a);
  wait_done();
}


static unsigned int goto_first_line(void)
{
  unsigned int dist;
  int16_t a, x, y;
  int is_done;
  unsigned int do_turn = 1;

  aversive_get_pos(&aversive_device, &a, &x, &y);

#if 1 /* true game */
  if (is_red) x = 780;
  else x = 3000 - 780;
#else
# warning TOREMOVE_FOR_MATCH
  if (is_red) x = 500;
  else x = 3000 - 500;
#endif

  aversive_goto_xy_abs(&aversive_device, x, y);

  while (1)
  {
    wait_abit();
    aversive_is_traj_done(&aversive_device, &is_done);
    if (is_done) break ;

    if (is_red) dist = sharp_read_fr();
    else dist = sharp_read_fl();

#define FIRST_PAWN_DIST 200
    if (dist <= FIRST_PAWN_DIST)
    {
      do_turn = 0;
      aversive_stop(&aversive_device);
      do_first_pawn();
      break ;
    }
  }

  return do_turn;
}


static void turn(void)
{
  const int16_t a = is_red ? -93 : 93;
  igreboard_open_gripper(&igreboard_device);
  aversive_turn(&aversive_device, a);
  wait_done();
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
    if ((direction == 0) && (y <= 500))
    {
      done_reason = DONE_REASON_POS;
      break ;
    }
    else if ((direction == 1) && (y >= 2100 - 350))
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

static inline unsigned int is_left_red(void)
{
  /* does not depend on the color */
  int16_t a, x, y;
  aversive_get_pos(&aversive_device, &a, &x, &y);
  y = 2100 - clamp_y(y);
  /* even left reds */
  if (direction == 0) return ((y / 350) & 1) == 0;
  else return ((y / 350) & 1) == 1;
}

static void do_putpawn_angle(int16_t d, int16_t a)
{
  aversive_turn(&aversive_device, a);
  wait_done();

  aversive_move_forward(&aversive_device, d + 20);
  wait_done();

  igreboard_open_gripper(&igreboard_device);
  swatch_wait_msecs(1000);

  aversive_move_forward(&aversive_device, -160);
  wait_done();

  aversive_turn(&aversive_device, -a);
  wait_done();
}

static inline void do_putpawn_left(int16_t d)
{
  do_putpawn_angle(d, 90);
}

static inline void do_putpawn_right(int16_t d)
{
  do_putpawn_angle(d, -90);
}

static void center_tile(void)
{
  unsigned int tilex, tiley;
  int16_t posa, posx, posy;
  int16_t midx, midy;
  int16_t d;

  aversive_get_pos(&aversive_device, &posa, &posx, &posy);

  if (direction == 0) orient_south();
  else orient_north();

  tilex = clamp_x(posx);
  tiley = clamp_y(posy);
  world_to_tile(&tilex, &tiley);

  midy = tiley * 350 + 175;
  if (midy > posy) d = -(midy - posy);
  else d = posy - midy;
  aversive_move_forward(&aversive_device, d);
  wait_done();
}

static void do_putpawn(int16_t d)
{
  center_tile();

  if (is_red)
  {
    if (is_left_red()) do_putpawn_left(d);
    else do_putpawn_right(d);
  }
  else
  {
    if (is_left_red()) do_putpawn_right(d);
    else do_putpawn_left(d);
  }
}


static unsigned int handle_done(void)
{
  unsigned int can_redo = 0;

 switch_done_reason:
  switch (done_reason)
  {
  default:
  case DONE_REASON_POS:
    {
#if 0
      if (direction == 0)
      {
	int16_t x;
	if (is_red) x = 3000 - 700;
	else x = 700;

	/* const int16_t x = is_red ? 1200 : 3000 - 1200; */
	/* const int16_t x = 3000 / 2; */
	aversive_goto_forward_xy_abs(&aversive_device, x, 550);
	if (wait_done_or_detected(x, 550)) goto switch_done_reason;

	orient_north();
	direction = 1;
	can_redo = 1;
	break ;
      }
      else if (direction == 1)
      {
	/* todo */
      }
#endif

      unit_bonus();

      goto case_done_reason_gameover;

      break ;
    }

  case DONE_REASON_GAMEOVER:
  case_done_reason_gameover:
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
      do_putpawn(0);
      can_redo = 1;
      break ;
    }

  case DONE_REASON_SWITCH:
    {
      const unsigned int msecs = swatch_get_msecs();
      igreboard_close_gripper(&igreboard_device);
      while ((swatch_get_msecs() - msecs) < 1000) ;
      do_putpawn(0);
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
  unsigned int do_turn;

  initialize();
  first_pos();
  wait_cord();

#if CONFIG_ENABLE_SONAR
  sonar_initialize();
#endif

  swatch_start_game();
  do_turn = goto_first_line();
  if (do_turn) turn();

 redo_move:
  move_until();
  can_redo = handle_done();
  if (can_redo) goto redo_move;
  while (1) asm("nop");
}
