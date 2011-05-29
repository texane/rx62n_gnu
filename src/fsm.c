#include "config.h"
#include "fsm.h"
#include "swatch.h"
#include "aversive.h"

#if CONFIG_ENABLE_SONAR
# include "sonar.h"
#endif


extern aversive_dev_t aversive_device;


void fsm_main(fsm_t* fsm)
{
#if CONFIG_ENABLE_SONAR
  /* TODO: avoidadvers_fsm_initialize(); */
#endif /* CONFIG_ENABLE_SONAR */

  while (fsm->is_done(fsm->data) == 0)
  {
#if CONFIG_ENABLE_SONAR
    if (sonar_is_detected())
    {
      fsm->preempt(fsm->data);
      break ;
    }
#endif /* CONFIG_ENABLE_SONAR */

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
