#include "config.h"
#include "fsm.h"

#if CONFIG_ENABLE_SWATCH
# include "swatch.h"
#endif

#if CONFIG_ENABLE_AVERSIVE
# include "aversive.h"
extern aversive_dev_t aversive_device;
#endif

#if CONFIG_ENABLE_SONAR
# include "sonar.h"
#endif


/* stack interface */

typedef struct fsm_stack
{
  unsigned int top; /* first avail */
  fsm_t* data[32];
} fsm_stack_t;

static fsm_stack_t stack = { 0, };

void fsm_push(fsm_t* fsm)
{
  stack.data[stack.top++] = fsm;
}


/* execution routines */

void fsm_execute_all(void)
{
  /* execute from the stack top */

  fsm_t* fsm;

  while (1) 
  {
    /* stack.top the first avail */
    if (stack.top == 0) break ;

    /* reread since it may have changed */
    fsm = stack.data[stack.top - 1];

#if CONFIG_ENABLE_SONAR
    if (sonar_is_detected())
    {
      fsm->preempt(fsm->data);
      /* TODO: avoidadvers_fsm_initialize(); */
      break ;
    }
#endif /* CONFIG_ENABLE_SONAR */

#if CONFIG_ENABLE_SWATCH
    if (swatch_is_game_over())
    {
      fsm->preempt(fsm->data);

#if CONFIG_ENABLE_AVERSIVE
      aversive_stop(&aversive_device);
      aversive_set_asserv(&aversive_device, 0);
      aversive_set_power(&aversive_device, 0);
#endif /* CONFIG_ENABLE_AVERSIVE */

      break ;
    }
#endif /* CONFIG_ENABLE_SWATCH */

    if (fsm->is_done(fsm->data))
    {
      /* pop the current and redo */
      --stack.top;
      continue ;
    }

    /* next insn */
    fsm->next(fsm->data);
  }
}

void fsm_execute_one(fsm_t* fsm)
{
  fsm_push(fsm);
  fsm_execute_all();
}
