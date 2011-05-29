#include "config.h"
#include "fsm.h"

void unit_firstpos(void)
{
  fsm_t fsm;
  firstpos_fsm_initialize(&fsm);
  fsm_main(&fsm);
}
