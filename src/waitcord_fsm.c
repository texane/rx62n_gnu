#include "config.h"
#include "fsm.h"
#include "swatch.h"
#include "igreboard.h"


extern igreboard_dev_t igreboard_device;


static unsigned int is_removed = 0;


static unsigned int cord_fsm_isdone(void* data)
{
  return is_removed == 1;
}


static void cord_fsm_next(void* data)
{
#if 0 /* TODO */
  if (cord_is_removed(&is_removed))
#endif
  {
    swatch_start_game();
  }
}


/* exported */

void cord_fsm_initialize(fsm_t* fsm)
{
  default_fsm_initialize(fsm);
  fsm->next = cord_fsm_next;
  fsm->is_done = cord_fsm_isdone;
  is_removed = 0;
}
