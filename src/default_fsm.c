#include "config.h"
#include "fsm.h"


/* do nothing fsm */

static unsigned int default_fsm_isdone(void* data)
{ return 1; }

static void default_fsm_preempt(void* data)
{}

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
