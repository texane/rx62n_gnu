#ifndef WAIT_H_INCLUDED
# define WAIT_H_INCLUDED


#include <stdint.h>


typedef enum wait_flag
{
  WAIT_FLAG_TRAJ = 0,
  WAIT_FLAG_SONAR,
  WAIT_FLAG_GAMEOVER,
  WAIT_FLAG_TIME,
  WAIT_FLAG_INVALID
} wait_flag_t;


typedef uint32_t wait_mask_t;

#define WAIT_MASK(__f) ((uint32_t)1 << WAIT_FLAG_ ## __f)
#define WAIT_MASK_OR2(__f0, __f1) ((uint32_t)1 << WAIT_FLAG_ ## __f)
#define WAIT_MASK_OR3(__f0, __f1, __f2) ((uint32_t)1 << WAIT_FLAG_ ## __f)
#define WAIT_MASK_ALL ((uint32_t)-1)

void wait_with_args(wait_mask_t*, uintptr_t);

static inline void wait_no_update(wait_mask_t m)
{ wait_with_args(&m, (uintptr_t)0); }

static inline void wait(wait_mask_t* m)
{ wait_with_args(m, (uintptr_t)0); }


#endif /* WAIT_H_INCLUDED */


/* wait.c */

#include "wait.h"

void wait(wait_mask_t* mask, uintptr_t args)
{
  /* todo: better to use function pointer array */

#define CONFIG_WAIT_DELAY 100 /* msecs */

  unsigned int sum = 0;

  while (1)
  {
    swatch_wait_msecs(CONFIG_WAIT_DELAY);
    sum += CONFIG_WAIT_DELAY;

    if (*mask & WAIT_MASK(GAMEOVER))
    {
      if (swatch_is_game_over())
      {
	*mask = WAIT_MASK(GAMEOVER);
	break ;
      }
    }

#if CONFIG_ENABLE_SONAR
    if (*mask & WAIT_MASK(SONAR))
    {
      sonar_schedule();
      if (sonar_is_detected())
      {
	*mask = WAIT_MASK(SONAR);
	break ;
      }
    }
#endif /* CONFIG_ENABLE_SONAR */

    if (*mask & WAIT_MASK(TRAJ))
    {
      int is_done;
      aversive_is_done(&aversive_device, &is_done);
      if (is_done)
      {
	*mask = WAIT_MASK(TRAJ);
	break ;
      }
    }

    if (*mask & WAIT_MASK(TIME))
    {
      if (sum >= ((unsigned int)args))
      {
	*mask = WAIT_MASK(TIME);
	break ;
      }
    }
  }
}

#if 0 /* usage */
int main(int ac, char** av)
{
  wait_mask_t mask;

  while (1)
  {
    aversive_move_forward(&aversive_device, 100);
    mask = WAIT_MASK_OR3(SONAR, GAMEOVER, TRAJ);
    wait(&mask, 0);

    if (mask == WAIT_MASK(TRAJ))
    {
      /* ... */
    }
    else /* typically need to stop the traj */
    {
      aversive_stop(&aversive_device);
      if (mask == WAIT_MASK(SONAR)) { /* ... */ }
      else if (mask == WAIT_MASK(GAMEOVER)) { /* ... */ }
    }
  }
}
#endif /* usage */
