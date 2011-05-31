#include "config.h"
#include <stdint.h>
#include "fsm.h"
#include "sharp.h"
#include "aversive.h"
#include "igreboard.h"
#include "swatch.h"
#include "sonar.h"


/* extern variables */

extern aversive_dev_t aversive_device;
extern igreboard_dev_t igreboard_device;

static unsigned int is_detected;

static unsigned int detected_fsm_isdone(void* fsm_)
{
  return is_detected == 0;
}

static void detected_fsm_next(void* fsm_)
{
  is_detected = sonar_is_detected();
}

/* exported */

void detected_fsm_initialize(fsm_t* fsm)
{
  default_fsm_initialize(fsm);
  fsm->next = detected_fsm_next;
  fsm->is_done = detected_fsm_isdone;

  is_detected = 1;
}
