#include "config.h"
#include <stdint.h>
#include "aversive.h"
#include "igreboard.h"
#include "swatch.h"
#include "fsm.h"


extern aversive_dev_t aversive_device;
extern igreboard_dev_t igreboard_device;


enum firstpos_state
{
  INIT = 0,
  MOVE_BACK_0,
  MOVE_FRONT_0,
  TURN_0,
  MOVE_BACK_1,
  MOVE_FRONT_1,
  TURN_1,
  WAIT_MSECS_THEN_BLOCK,
  WAIT_BLOCK,
  WAIT_TRAJ,
  DONE
};

typedef struct firstpos_fsm
{
  enum firstpos_state state;
  enum firstpos_state next_state;

  unsigned int is_red;

  /* last read pos */
  int16_t posx;
  int16_t posy;

  /* last swatch msecs */
  unsigned int msecs;

} firstpos_fsm_t;


static unsigned int firstpos_fsm_isdone(void* data)
{
  firstpos_fsm_t* const fsm = data;
  return fsm->state == DONE;
}


static void firstpos_fsm_next(void* data)
{
  firstpos_fsm_t* const fsm = data;
  unsigned int msecs;
  unsigned int map;
  int16_t posa;
  int16_t posx;
  int16_t posy;
  int16_t dx;
  int16_t dy;
  int isdone;

  switch (fsm->state)
  {
  case INIT:
    /* aversive_set_speed(&aversive_device, 500, 500); */
    fsm->state = MOVE_BACK_0;
    igreboard_get_color_switch(&igreboard_device, &fsm->is_red);
    break ;

  case MOVE_BACK_0:
    aversive_move_forward(&aversive_device, -150);
    fsm->msecs = swatch_get_msecs();
    fsm->state = WAIT_MSECS_THEN_BLOCK;
    fsm->next_state = MOVE_FRONT_0;
    break ;

  case MOVE_FRONT_0:
    if (fsm->is_red) aversive_set_pos(&aversive_device, 0, 97, 0);
    else aversive_set_pos(&aversive_device, 180, 3000 - 97, 0);
    aversive_move_forward(&aversive_device, 150);
    fsm->next_state = TURN_0;
    fsm->state = WAIT_TRAJ;
    break ;

  case TURN_0:
    if (fsm->is_red) aversive_turn(&aversive_device, -90);
    else aversive_turn(&aversive_device, 90);
    fsm->next_state = MOVE_BACK_1;
    fsm->state = WAIT_TRAJ;
    break ;

  case MOVE_BACK_1:
    aversive_move_forward(&aversive_device, -100);
    fsm->msecs = swatch_get_msecs();
    fsm->state = WAIT_MSECS_THEN_BLOCK;
    fsm->next_state = MOVE_FRONT_1;
    break ;

  case MOVE_FRONT_1:
    aversive_get_pos(&aversive_device, &posa, &posx, &posy);
    aversive_set_pos(&aversive_device, posa, posx, 2100 - 160);
    aversive_move_forward(&aversive_device, 150);
    fsm->next_state = TURN_1;
    fsm->state = WAIT_TRAJ;
    break ;

  case TURN_1:
    if (fsm->is_red) aversive_turn(&aversive_device, 90);
    else aversive_turn(&aversive_device, -90);
    fsm->next_state = DONE;
    fsm->state = WAIT_TRAJ;
    break ;

  case WAIT_MSECS_THEN_BLOCK:
    /* wait for 1 msecs */
    msecs = swatch_get_msecs();
    if ((msecs - fsm->msecs) < 1000) break ;
    fsm->msecs = msecs;

    aversive_get_pos(&aversive_device, &posa, &fsm->posx, &fsm->posy);
    fsm->state = WAIT_BLOCK;

    break ;

  case WAIT_BLOCK:
    /* poll every 300 ms */
    msecs = swatch_get_msecs();
    if ((msecs - fsm->msecs) < 300) break ;
    fsm->msecs = msecs;

    posx = fsm->posx;
    posy = fsm->posy;
    aversive_get_pos(&aversive_device, &posa, &fsm->posx, &fsm->posy);

    igreboard_get_back_switches(&igreboard_device, &map);
    if (map) goto on_blocked;

    dx = fsm->posx - posx;
    if (posx > fsm->posx) dx = posx - fsm->posx;

    dy = fsm->posy - posy;
    if (posy > fsm->posy) dy = posy - fsm->posy;

    /* consider 10mm as blocked */
    if ((dx + dy) >= 10) break ;

  on_blocked:
    aversive_stop(&aversive_device);
    fsm->state = fsm->next_state;

    break ;

  case WAIT_TRAJ:
    aversive_is_traj_done(&aversive_device, &isdone);
    if (isdone == 0) break ;
    fsm->state = fsm->next_state;
    break ;

  default:
  case DONE:
    break ;
  }
}

/* exported */

void firstpos_fsm_initialize(fsm_t* fsm)
{
  static firstpos_fsm_t data;

  default_fsm_initialize(fsm);
  fsm->next = firstpos_fsm_next;
  fsm->is_done = firstpos_fsm_isdone;

  data.state = INIT;

  fsm->data = (void*)&data;
}
