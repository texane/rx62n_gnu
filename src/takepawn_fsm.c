#include "config.h"
#include "fsm.h"
#include "sharp.h"
#include "aversive.h"
#include "igreboard.h"
#include "swatch.h"
#include "lcd.h"


/* extern variables */

extern aversive_dev_t aversive_device;
extern igreboard_dev_t igreboard_device;


/* take pawn finite state machine */

typedef struct takepawn_fsm
{
  /* current state */
  enum
  {
    INIT = 0,
    OPEN_GRIPPER,
    SEARCH,
    CENTER,
    CENTER_2,
    MOVE,
    GRIPPER_SWITCH,
    CLOSE_GRIPPER,
    WAIT_TRAJ,
    WAIT_TRAJ_OR_GRIPPER_SWITCH,
    WAIT_GRIPPER,
    FAILED,
    DONE
  } state;

  unsigned int center2_pass;

  /* store the previous or next state */
  unsigned int prev_state;

  unsigned int saved_dist;

  /* front left, right sharps */
  unsigned int fl;
  unsigned int fr;

  /* angle */
  int alpha;

  /* track time from last operation */
  unsigned int msecs;

  /* operation status */
  int status;

} takepawn_fsm_t;


static unsigned int takepawn_fsm_isdone(void* data)
{
  takepawn_fsm_t* const fsm = data;
  return fsm->state == DONE;
}

static inline unsigned int min(unsigned int a, unsigned int b)
{
  return a < b ? a : b;
}

static void takepawn_fsm_next(void* data)
{
  takepawn_fsm_t* const fsm = data;

  switch (fsm->state)
  {
  case INIT:
    {
      /* open the gripper and search */

      lcd_string(3, 0, "init        ");

      fsm->state = OPEN_GRIPPER;
      break ;
    }

  case OPEN_GRIPPER:
    {
      lcd_string(3, 0, "open_gripper");

      igreboard_open_gripper(&igreboard_device);
      fsm->msecs = swatch_get_msecs();
      fsm->prev_state = SEARCH;
      fsm->state = WAIT_GRIPPER;
      break ;
    }

  case SEARCH:
    {
      lcd_string(3, 0, "search      ");

      /* turn looking for the something */
      fsm->fl = sharp_read_fl();
      fsm->fr = sharp_read_fr();

      lcd_uint16(4, 0, (uint16_t)fsm->fl);
      lcd_uint16(4, 30, (uint16_t)fsm->fr);

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

  case CENTER:
    {
      unsigned int d;

      lcd_string(3, 0, "center       ");

      /* turn until the 2 read approx the same distance */
      fsm->fl = sharp_read_fl();
      fsm->fr = sharp_read_fr();

      /* in case of perfectly centered, we save the previous distance */
      if (fsm->fl != (unsigned int)-1) fsm->saved_dist = fsm->fl;

      lcd_uint16(4, 0, (uint16_t)fsm->fl);
      lcd_uint16(4, 30, (uint16_t)fsm->fr);

#if 0 /* perfectly centered? */
      if ((fsm->fl == (unsigned int)-1) && (fsm->fl == fsm->fr))
      {
	/* perfectly centered? */
	fsm->fl = 0;
	fsm->fr = 0;
      }
#endif

      if (fsm->fl > fsm->fr) d = fsm->fl - fsm->fr;
      else d = fsm->fr - fsm->fl;

      if (d < 60)
      {
	fsm->state = MOVE;
	break ;
      }

      /* angle roughly proportional to distance */
      if (d > 100) fsm->alpha = 8;
      else if (d > 75) fsm->alpha = 7;
      else fsm->alpha = 6;

      if (fsm->fl > fsm->fr) fsm->alpha *= -1;

      aversive_turn(&aversive_device, fsm->alpha);

      fsm->center2_pass = 0;
      fsm->prev_state = CENTER_2;
      fsm->state = WAIT_TRAJ;

      break ;
    }

  case CENTER_2:
    {
      unsigned int d;

      lcd_string(3, 0, "center2     ");

      /* turn until the 2 read approx the same distance */
      fsm->fl = sharp_read_fl();
      fsm->fr = sharp_read_fr();

      /* in case of perfectly centered, we save the previous distance */
      if (fsm->fl != (unsigned int)-1) fsm->saved_dist = fsm->fl;

      if (++fsm->center2_pass == 20)
      {
	fsm->state = MOVE;
	break ;
      }

      if (fsm->fl > fsm->fr) d = fsm->fl - fsm->fr;
      else d = fsm->fr - fsm->fl;

      if (d < 60)
      {
	fsm->state = MOVE;
	break ;
      }

      /* angle roughly proportional to distance */
      if (d > 100) fsm->alpha = 4;
      else fsm->alpha = 3;

      if (fsm->fl > fsm->fr) fsm->alpha *= -1;

      aversive_turn(&aversive_device, fsm->alpha);
      fsm->prev_state = CENTER_2;
      fsm->state = WAIT_TRAJ;

      break ;
    }
  
  case MOVE:
    {
      /* assume centered, move forward */

      lcd_string(3, 0, "move     ");

      /* if perfectly centered */
      if (fsm->fl == (unsigned int)-1)
      {
	if (fsm->saved_dist == 0) fsm->saved_dist = 50;

	aversive_move_forward(&aversive_device, fsm->saved_dist);
	
	/* fsm->prev_state = CLOSE_GRIPPER; */
	fsm->prev_state = CENTER;
      }
      else /* not perfectly centered */
      {
	aversive_move_forward(&aversive_device, min(45, fsm->fl));
	if (fsm->fl < 45) fsm->prev_state = CLOSE_GRIPPER;
	else fsm->prev_state = CENTER;
      }

      /* moving may invalidate center */
      fsm->state = WAIT_TRAJ_OR_GRIPPER_SWITCH;

      break ;
    }

  case GRIPPER_SWITCH:
    {
      lcd_string(3, 0, "gripper_switch");

      /* move until the grip switch pressed or distance reached */
      /* TODO aversive_move_forward(&aversive_device, 10 + (fsm->fl + fsm->fr) / 2); */
      aversive_move_forward(&aversive_device, 100);
      fsm->state = WAIT_TRAJ_OR_GRIPPER_SWITCH;
      break ;
    }

  case CLOSE_GRIPPER:
    {
      lcd_string(3, 0, "close_gripper");

      igreboard_close_gripper(&igreboard_device);
      fsm->msecs = swatch_get_msecs();
      fsm->prev_state = DONE;
      fsm->state = WAIT_GRIPPER;
      break ;
    }

  case WAIT_TRAJ:
    {
      int isdone;

      lcd_string(3, 0, "wait_traj     ");

      aversive_is_traj_done(&aversive_device, &isdone);
      if (isdone == 1)
	fsm->state = fsm->prev_state;
      break ;
    }

  case WAIT_TRAJ_OR_GRIPPER_SWITCH:
    {
      unsigned int is_pushed;
      int is_done;

      lcd_string(3, 0, "wait_traj_or  ");

      igreboard_get_gripper_switch(&igreboard_device, &is_pushed);
      aversive_is_traj_done(&aversive_device, &is_done);

      if (is_pushed)
      {
	/* stop aversive trajectory */
	aversive_stop(&aversive_device);
	fsm->state = CLOSE_GRIPPER;
      }
      else if (is_done)
      {
	/* recenter if switch was not pressed */
	fsm->state = fsm->prev_state;
      }

      break ;
    }

  case WAIT_GRIPPER:
    {
      lcd_string(3, 0, "wait_gripper");

      /* wait for 800 msecs */
      if ((swatch_get_msecs() - fsm->msecs) >= 800)
	fsm->state = fsm->prev_state;

      break ;
    }

  case FAILED:
    {
      lcd_string(3, 0, "failed      ");
      fsm->status = -1;
      fsm->state = DONE;
      break ;
    }

  default:
  case DONE:
    {
      lcd_string(3, 0, "done        ");
      break ;
    }
  }
}


/* exported */

void takepawn_fsm_initialize(fsm_t* fsm)
{
  static takepawn_fsm_t data;

  default_fsm_initialize(fsm);
  fsm->next = takepawn_fsm_next;
  fsm->is_done = takepawn_fsm_isdone;

  data.state = INIT;
  data.fl = 0;
  data.fr = 0;
  data.alpha = 0;
  data.status = 0;

  fsm->data = (void*)&data;
}
