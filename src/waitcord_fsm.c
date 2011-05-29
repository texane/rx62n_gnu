#include "config.h"
#include "fsm.h"
#include "igreboard.h"


extern igreboard_dev_t igreboard_device;


static unsigned int cord_fsm_isdone(void* data)
{
  /* TODO */
  return 0;
}


static void cord_fsm_next(void* data)
{
  /* TODO */
}


/* exported */

void cord_fsm_initialize(fsm_t* fsm)
{
  default_fsm_initialize(fsm);
  fsm->next = cord_fsm_next;
  fsm->is_done = cord_fsm_isdone;
}
