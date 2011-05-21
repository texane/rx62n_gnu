#include "fsm.h"
#include "sharp.h"
#include "aversive.h"


/* extern variables */

extern aversive_dev_t aversive_device;


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


static void takepawn_fsm_preempt(void* data)
{}


static void takepawn_fsm_restart(void* data)
{}

/* exported */

void takepawn_fsm_initialize(fsm_t* fsm)
{
  static takepawn_fsm_t data;

  fsm->next = takepawn_fsm_next;
  fsm->is_done = takepawn_fsm_isdone;
  fsm->preempt = takepawn_fsm_preempt;
  fsm->restart = takepawn_fsm_restart;

  data.state = SEARCH;
  data.prev_state = SEARCH;
  data.fl = 0;
  data.fr = 0;
  data.alpha = 0;

  fsm->data = (void*)&data;
}
