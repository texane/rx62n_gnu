#include "fsm.h"
#include "sharp.h"
#include "aversive.h"
#include "igreboard.h"
#include "lcd.h"


/* extern variables */

extern aversive_dev_t aversive_device;
extern igreboard_dev_t igreboard_device;


/* take pawn finite state machine */

typedef struct takepawn_fsm
{
  /* current state */
  enum
  {
    SEARCH = 0,
    CENTER,
    MOVE,
    SWITCH,
    GRIP,
    WAIT_TRAJ,
    WAIT_TRAJ_OR_SWITCH,
    WAIT_GRIP,
    DONE
  } state;

  /* store the previous state */
  unsigned int prev_state;

  /* front left, right sharps */
  unsigned int fl;
  unsigned int fr;

  /* angle */
  int alpha;

  /* gripper delay */
  unsigned int gripper_delay;

} takepawn_fsm_t;


static unsigned int takepawn_fsm_isdone(void* data)
{
  takepawn_fsm_t* const fsm = data;
  return fsm->state == DONE;
}

static inline unsigned int min(unsigned int a, unsigned int b)
{
  return a < b ? a : b;
}

static inline char nibble_to_ascii(uint8_t value)
{
  if (value >= 0xa) return 'a' + value - 0xa;
  return '0' + value;
}

static const char* uint16_to_string(uint16_t value)
{
  static char buf[8];
  unsigned int i;

  for (i = 0; i < 4; ++i, value >>= 4)
    buf[4 - i - 1] = nibble_to_ascii(value & 0xf);
  buf[i] = 0;

  return buf;
}

static inline void print_uint16
(unsigned int row, unsigned int col, uint16_t value)
{
  lcd_string((uint8_t)row, (uint8_t)col, uint16_to_string(value));
}

static void takepawn_fsm_next(void* data)
{
  takepawn_fsm_t* const fsm = data;

  switch (fsm->state)
  {
  case SEARCH:
    {
      lcd_string(3, 0, "search");

      /* turn looking for the something */
      fsm->fl = sharp_read_fl();
      fsm->fr = sharp_read_fr();

      print_uint16(4, 0, (uint16_t)fsm->fl);
      print_uint16(4, 30, (uint16_t)fsm->fr);

#define PAWN_DIST 200
      if (min(fsm->fl, fsm->fr) < PAWN_DIST)
      {
	fsm->state = CENTER;
	break ;
      }

      /* continue turning */
      aversive_turn(&aversive_device, fsm->alpha);
      fsm->prev_state = SEARCH;
      fsm->state = WAIT_TRAJ;

      break ;
    }

  case CENTER:
    {
      unsigned int d;

      lcd_string(3, 0, "center");

      /* turn until the 2 read approx the same distance */
      fsm->fl = sharp_read_fl();
      fsm->fr = sharp_read_fr();

      print_uint16(4, 0, (uint16_t)fsm->fl);
      print_uint16(4, 30, (uint16_t)fsm->fr);

      if (fsm->fl > fsm->fr) d = fsm->fl - fsm->fr;
      else d = fsm->fr - fsm->fl;

      if (d < 30)
      {
	fsm->state = MOVE;
	break ;
      }

      /* todo: angle should be proportional to d */
      if (fsm->fl > fsm->fr) fsm->alpha = 8;
      else fsm->alpha = -8;

      aversive_turn(&aversive_device, fsm->alpha);
      fsm->prev_state = CENTER;
      fsm->state = WAIT_TRAJ;

      break ;
    }
  
  case MOVE:
    {
      /* assume centered, move forward if not too near */

      lcd_string(3, 0, "move");
      
      if (fsm->fl <= 45)
      {
	fsm->state = SWITCH;
	break ;
      }

      aversive_move_forward(&aversive_device, 45);

      /* moving may invalidate center */
      fsm->prev_state = CENTER;
      fsm->state = WAIT_TRAJ;

      break ;
    }

  case SWITCH:
    {
      lcd_string(3, 0, "switch");

      /* move until the grip switch pressed or distance reached */
      /* TODO aversive_move_forward(&aversive_device, 10 + (fsm->fl + fsm->fr) / 2); */
      aversive_move_forward(&aversive_device, 100);
      fsm->state = WAIT_TRAJ_OR_SWITCH;
      break ;
    }

  case GRIP:
    {
      lcd_string(3, 0, "grip");

      igreboard_close_gripper(&igreboard_device);
      fsm->gripper_delay = 0;
      fsm->state = WAIT_GRIP;
      break ;
    }

  case WAIT_TRAJ:
    {
      int isdone;

      lcd_string(3, 0, "wait_traj");

      aversive_is_traj_done(&aversive_device, &isdone);
      if (isdone == 1)
	fsm->state = fsm->prev_state;
      break ;
    }

  case WAIT_TRAJ_OR_SWITCH:
    {
      unsigned int is_pushed;
      int is_done;

      lcd_string(3, 0, "wait_traj_or_switch");

#if 0 /* TODO: not yet physically present */
      igreboard_get_gripper_switch(&igreboard_device, &is_pushed);
#else
      is_pushed = 0;
#endif
      aversive_is_traj_done(&aversive_device, &is_done);

      if (is_pushed)
      {
	/* stop aversive trajectory */
	aversive_stop(&aversive_device);
	fsm->state = GRIP;
      }
      else if (is_done)
      {
	fsm->state = GRIP;
      }

      break ;
    }

  case WAIT_GRIP:
    {
      lcd_string(3, 0, "wait_grip");

      if (++fsm->gripper_delay == 100)
	fsm->state = DONE;
      break ;
    }

  default:
  case DONE:
    {
      lcd_string(3, 0, "done");
      break ;
    }
  }
}


/* exported */

void takepawn_fsm_initialize(fsm_t* fsm)
{
  static takepawn_fsm_t data;

  default_fsm_initialize(fsm);
  fsm->next = takepawn_fsm_next;
  fsm->is_done = takepawn_fsm_isdone;

  data.state = SEARCH;
  data.prev_state = SEARCH;
  data.fl = 0;
  data.fr = 0;
  data.alpha = 0;

  fsm->data = (void*)&data;
}
