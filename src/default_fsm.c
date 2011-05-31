#include "config.h"
#include "fsm.h"
#include "aversive.h"
#include "lcd.h"


extern aversive_dev_t aversive_device;


/* do nothing fsm */

static unsigned int default_fsm_isdone(void* data)
{
  return 1;
}

static void default_fsm_preempt(void* data)
{
  /* this should be handled by every fsm... */

  int is_done;
  aversive_is_traj_done(&aversive_device, &is_done);

  if (is_done == 0)
  {
    aversive_stop(&aversive_device);
    lcd_string(3, 0, "preempted");
  }
}

static void default_fsm_restart(void* data)
{}

static void default_fsm_next(void* data)
{}

void default_fsm_initialize(fsm_t* fsm)
{
  fsm->next = default_fsm_next;
  fsm->is_done = default_fsm_isdone;
  fsm->preempt = default_fsm_preempt;
  fsm->restart = default_fsm_restart;
  fsm->data = 0;
}
