#include "config.h"
#include <stdint.h>
#include "fsm.h"
#include "sharp.h"
#include "aversive.h"
#include "igreboard.h"
#include "swatch.h"


/* extern variables */

extern aversive_dev_t aversive_device;
extern igreboard_dev_t igreboard_device;


typedef struct leavestart_fsm
{
  enum
  {
    INIT,
    MOVE_0,
    MOVE_1,
    TURN_0,
    TURN_1,
    WAIT_TRAJ,
    DONE
  } state;

  unsigned int msecs;
  unsigned int is_red;
  unsigned int next_state;

} leavestart_fsm_t;


static unsigned int leavestart_fsm_isdone(void* fsm_)
{
  leavestart_fsm_t* const fsm = fsm_;
  return fsm->state == DONE;
}

static void leavestart_fsm_next(void* fsm_)
{
  leavestart_fsm_t* const fsm = fsm_;

  switch (fsm->state)
  {
  case INIT:
    {
      /* color related init */
      igreboard_get_color_switch(&igreboard_device, &fsm->is_red);
      fsm->state = MOVE_0;
      break ;
    }

  case MOVE_0:
    {
      aversive_move_forward(&aversive_device, 350);
      fsm->msecs = swatch_get_msecs();
      fsm->state = WAIT_TRAJ;
      fsm->next_state = TURN_0;
      break ;
    }

  case TURN_0:
    {
      const int16_t alpha = fsm->is_red ? -95 : 95;
      aversive_turn(&aversive_device, alpha);
      fsm->msecs = swatch_get_msecs();
      fsm->state = WAIT_TRAJ;
      fsm->next_state = MOVE_1;
      break ;
    }

  case MOVE_1:
    {
      aversive_move_forward(&aversive_device, 300);
      fsm->msecs = swatch_get_msecs();
      fsm->state = WAIT_TRAJ;
      fsm->next_state = TURN_1;
      break ;
    }

  case TURN_1:
    {
      const int16_t alpha = fsm->is_red ? 5 : -5;
      aversive_turn(&aversive_device, alpha);
      fsm->msecs = swatch_get_msecs();
      fsm->state = WAIT_TRAJ;
      fsm->next_state = DONE;
      break ;
    }

  case WAIT_TRAJ:
    {
      const unsigned int msecs = swatch_get_msecs();
      int isdone;
      if ((msecs - fsm->msecs) < 200) break ;
      fsm->msecs = msecs;
      aversive_is_traj_done(&aversive_device, &isdone);
      if (isdone == 1) fsm->state = fsm->next_state;
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

void leavestart_fsm_initialize(fsm_t* fsm)
{
  static leavestart_fsm_t data;

  default_fsm_initialize(fsm);
  fsm->next = leavestart_fsm_next;
  fsm->is_done = leavestart_fsm_isdone;

  data.state = INIT;

  fsm->data = (void*)&data;
}
