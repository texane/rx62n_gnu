#include "config.h"
#include "fsm.h"
#include "swatch.h"
#include "igreboard.h"


extern igreboard_dev_t igreboard_device;


static unsigned int prev_msecs = (unsigned int)-1;
static unsigned int is_removed = 0;


static unsigned int cord_fsm_isdone(void* data)
{
  return is_removed;
}


static void cord_fsm_next(void* data)
{
  unsigned int msecs;
  unsigned int is_pushed;

  if (prev_msecs == -1) prev_msecs = swatch_get_msecs();

  msecs = swatch_get_msecs();
  if ((msecs - prev_msecs) < 100) return ;
  prev_msecs = msecs;
  igreboard_get_cord_switch(&igreboard_device, &is_pushed);

  if (is_pushed == 0)
  {
    is_removed = 1;
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
