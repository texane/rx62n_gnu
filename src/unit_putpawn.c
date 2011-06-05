#include "config.h"
#include "fsm.h"

void unit_putpawn(void)
{
  fsm_t fsm;
  putpawn_fsm_initialize(&fsm);
  fsm_execute_one(&fsm);
}
