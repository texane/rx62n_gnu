#include "config.h"
#include <stdint.h>
#include "fsm.h"
#include "sharp.h"
#include "aversive.h"
#include "igreboard.h"
#include "swatch.h"
#include "lcd.h"


/* extern variables */

extern aversive_dev_t aversive_device;
extern igreboard_dev_t igreboard_device;


typedef struct latpawn_fsm
{
  /* current state */
  enum
  {
    INIT = 0,
    MOVE,
    TURN,
    WAIT_TRAJ_OR_PAWN_OR_POS,
    WAIT_TRAJ,
    DONE
  } state;

  unsigned int is_red;

  unsigned int next_state;
  unsigned int sharp_msecs;
  unsigned int msecs;
  unsigned int is_pawn;

} latpawn_fsm_t;


static unsigned int latpawn_fsm_isdone(void* data)
{
  latpawn_fsm_t* const fsm = data;
  return fsm->state == DONE;
}

/* static inline unsigned int min(unsigned int a, unsigned int b) */
/* { */
/*   return a < b ? a : b; */
/* } */

static void latpawn_fsm_next(void* data)
{
  latpawn_fsm_t* const fsm = data;

  switch (fsm->state)
  {
  case INIT:
    {
      fsm->is_pawn = 0;
      igreboard_get_color_switch(&igreboard_device, &fsm->is_red);
      fsm->sharp_msecs = swatch_get_msecs();
      fsm->msecs = fsm->sharp_msecs;
      fsm->state = MOVE;
      break ;
    }

  case MOVE:
    {
      aversive_move_forward(&aversive_device, 600);
      fsm->msecs = swatch_get_msecs();
      fsm->sharp_msecs = fsm->msecs;
      fsm->state = WAIT_TRAJ_OR_PAWN_OR_POS;
      break ;
    }

  case TURN:
    {
      if (fsm->is_red) aversive_turn(&aversive_device, -90);
      else aversive_turn(&aversive_device, 90);
      fsm->msecs = swatch_get_msecs();
      fsm->next_state = DONE;
      fsm->state = WAIT_TRAJ;
      break ;
    }

  case WAIT_TRAJ_OR_PAWN_OR_POS:
    {
      const unsigned int msecs = swatch_get_msecs();

      /* check pawn every 5 msecs */
      if ((msecs - fsm->sharp_msecs) >= 5)
      {
	/* front, back disitances */
	unsigned int b, f;

	/* update sharp msecs */
	fsm->sharp_msecs = msecs;

	/* red looks at right, blue at left */
	if (fsm->is_red)
	{
	  b = sharp_read_rb();
	  f = sharp_read_rf();
	}
	else
	{
	  b = sharp_read_lb();
	  f = sharp_read_lf();
	}

#define PAWN_DIST 200
	if ((f < PAWN_DIST) && (b < PAWN_DIST))
	{
	  aversive_stop(&aversive_device);

	  fsm->is_pawn = 1;
	  fsm->state = TURN;

	  break ;
	}

	break ;
      }

      /* check traj every 20 msecs */
      if (msecs - fsm->msecs >= 20)
      {
	int16_t a, x, y;
	int isdone;

	/* update msecs */
	fsm->msecs = msecs;

	/* trajectory done */
	aversive_is_traj_done(&aversive_device, &isdone);
	if (isdone)
	{
	  fsm->state = fsm->next_state;
	  break ;
	}

	/* final position reached */
	aversive_get_pos(&aversive_device, &a, &x, &y);
	if (y == 1800)
	{
	  aversive_stop(&aversive_device);
	  fsm->state = DONE;
	  break ;
	}
      }

      break;
    }

  case WAIT_TRAJ:
    {
      const unsigned int msecs = swatch_get_msecs();
      if ((msecs - fsm->msecs) >= 20)
      {
	int isdone;

	/* update msecs */
	fsm->msecs = msecs;

	aversive_is_traj_done(&aversive_device, &isdone);
	if (isdone) fsm->state = fsm->next_state;
      }

      break ;
    }

  default:
  case DONE:
    {
      break ;
    }
  }
}

/* exported */

void latpawn_fsm_initialize(fsm_t* fsm)
{
  static latpawn_fsm_t data;

  default_fsm_initialize(fsm);
  fsm->next = latpawn_fsm_next;
  fsm->is_done = latpawn_fsm_isdone;

  data.state = INIT;
  data.is_pawn = 0;

  fsm->data = (void*)&data;
}

unsigned int latpawn_is_pawn(void* fsm_)
{
  latpawn_fsm_t* const fsm = fsm_;
  return fsm->is_pawn;
}
