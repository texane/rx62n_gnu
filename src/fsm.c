#include "config.h"
#include "fsm.h"
#include "swatch.h"
#include "aversive.h"


extern aversive_dev_t aversive_device;


void fsm_main(fsm_t* fsm)
{
  while (fsm->is_done(fsm->data) == 0)
  {
    if (swatch_is_game_over())
    {
      fsm->preempt(fsm->data);
      break ;
    }

    fsm->next(fsm->data);
  }

  /* game is over */
  aversive_stop(&aversive_device);
  aversive_set_asserv(&aversive_device, 0);
  aversive_set_power(&aversive_device, 0);
}
