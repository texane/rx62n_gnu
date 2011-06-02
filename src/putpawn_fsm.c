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

typedef struct putpawn_fsm
{
  /* current state */
  enum
  {
    INIT = 0,
    FIND_TILE,
    TURN,
    MOVE_FRONT,
    OPEN_GRIPPER,
    MOVE_BACK,
    WAIT_TRAJ,
    DONE
  } state;

  /* store the previous or next state */
  unsigned int next_state;

  /* track time from last operation */
  unsigned int msecs;

} putpawn_fsm_t;


static unsigned int putpawn_fsm_isdone(void* data)
{
  takepawn_fsm_t* const fsm = data;
  return fsm->state == DONE;
}

static void putpawn_fsm_next(void* fsm_)
{
  putpawn_fsm_t* const fsm = fsm_;

  switch (fsm->state)
  {
  case INIT:
    break ;

  case FIND_TILE:
    break ;

  case TURN:
    break ;

  case MOVE_FRONT:
    break ;

  case OPEN_GRIPPER:
    break ;

  case MOVE_BACK:
    break ;

  case WAIT_TRAJ:
    break ;
    
  default:
  case DONE:
    break ;
}


/* exported */

void putpawn_fsm_initialize(fsm_t* fsm)
{
  static putpawn_fsm_t data;

  default_fsm_initialize(fsm);
  fsm->next = putpawn_fsm_next;
  fsm->is_done = putpawn_fsm_isdone;

  data.state = INIT;

  fsm->data = (void*)&data;
}
