#include "config.h"
#include "fsm.h"

void unit_takepawn(void)
{
  fsm_t fsm;
  takepawn_fsm_initialize(&fsm);
  fsm_main(&fsm);
}
