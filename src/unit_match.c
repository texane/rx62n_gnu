#include "config.h"
#include "fsm.h"


void unit_match(void)
{
  static fsm_t waitcord_fsm;
  static fsm_t firstpos_fsm;

  /* bootstrap */
  /* fsm_push(wanderer_fsm); */
  fsm_push(emptyarea_fsm);
  fsm_push(leavestart_fsm);
  fsm_push(waitcord_fsm);
  fsm_push(firstpos_fsm);

  fsm_execute_all();
}
