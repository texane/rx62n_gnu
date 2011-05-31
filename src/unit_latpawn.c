#include "config.h"
#include "fsm.h"

void unit_latpawn(void)
{
  fsm_t fsm;
  latpawn_fsm_initialize(&fsm);
  fsm_execute_one(&fsm);
}

