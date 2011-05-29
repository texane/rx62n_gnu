#include "config.h"
#include "aversive.h"
#include "fsm.h"


extern aversive_dev_t aversive_device;


typedef struct firstpos_fsm
{
  /* current state */
  enum
  {
    MOVE_BACK = 0,
    WAIT_TRAJ_OR_BLOCK,
    DONE
  } state;

} firstpos_fsm_t;


static unsigned int firstpos_fsm_isdone(void* data)
{
  firstpos_fsm_t* const fsm = data;
  return fsm->state == DONE;
}


static void firstpos_fsm_next(void* data)
{
  /* firstpos_fsm_t* const fsm = data; */

#if 0

	set_speed(50, 50);
	avance_s(-100);
	time_wait_ms(1000);
	for(;;) {
		double a, x, y;
		double aa, xx, yy;
		double moved;
		get_pos(&a, &x, &y);
		time_wait_ms(300);
		get_pos(&aa, &xx, &yy);
		moved = sqrt((xx-x)*(xx-x) + (yy-y)*(yy-y));
		if (moved < 0.2)
			break;
	}		
	stop();
	time_wait_ms(1000);
	send_msg(CARTE_ASSERV, 'r', 0); // Asserv off;
	set_pwm(-40, -40);
	time_wait_ms(200);
	for(;;) {
		double a, x, y;
		double aa, xx, yy;
		double moved;
		get_pos(&a, &x, &y);
		time_wait_ms(100);
		get_pos(&aa, &xx, &yy);
		moved = sqrt((xx-x)*(xx-x) + (yy-y)*(yy-y));
		if (moved < 0.1)
			break;
	}		
	set_pwm(0, 0);
	time_wait_ms(1000);
	set_pos(0, 0, 0);
	
	send_msg(CARTE_ASSERV, 'r', 1); // Asserv on;
	goto_axy(0, 10, 0);
	while (!traj_finie()) time_wait_ms(100);
		
	tourne(-90);
	while (!traj_finie()) time_wait_ms(100);
	time_wait_ms(1000);
	
	avance_s(-100);
	time_wait_ms(1000);
	for(;;) {
		double a, x, y;
		double aa, xx, yy;
		double moved;
		get_pos(&a, &x, &y);
		time_wait_ms(300);
		get_pos(&aa, &xx, &yy);
		moved = sqrt((xx-x)*(xx-x) + (yy-y)*(yy-y));
		if (moved < 0.2)
			break;
	}		
	stop();
	time_wait_ms(1000);
	set_pos(0, 0, 0);

	goto_axy_dbg(45, 20, 0);


#endif

}


static void firstpos_fsm_preempt(void* data)
{
  /* firstpos_fsm_t* const fsm = data; */
}


static void firstpos_fsm_restart(void* data)
{
  /* firstpos_fsm_t* const fsm = data; */
}


/* exported */

void firstpos_fsm_initialize(fsm_t* fsm)
{
  static firstpos_fsm_t data;

  fsm->next = firstpos_fsm_next;
  fsm->is_done = firstpos_fsm_isdone;
  fsm->preempt = firstpos_fsm_preempt;
  fsm->restart = firstpos_fsm_restart;

  data.state = MOVE_BACK;

  fsm->data = (void*)&data;
}

