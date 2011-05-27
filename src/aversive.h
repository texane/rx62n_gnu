/*
** Made by fabien le mentec <texane@gmail.com>
*/



#ifndef AVERSIVE_H_INCLUDED
# define AVERSIVE_H_INCLUDED


#include <stdint.h>


#define CONFIG_USE_I2C 0
#define CONFIG_USE_CAN 1


#if CONFIG_USE_I2C

#include "i2c.h"

typedef struct aversive_dev
{
  i2c_dev_t i2c_dev;
} aversive_dev_t;

#define AVERSIVE_DEV_INITIALIZER { I2C_DEV_INITIALIZER }

#elif CONFIG_USE_CAN

/* dont forward decl, include to force config */
#include "can_config.h"
#include "can.h"

typedef struct aversive_dev
{
  can_dev_t* can_dev;
} aversive_dev_t;

#define AVERSIVE_DEV_INITIALIZER { 0 }

#endif


/* trajectory status */
#define AVERSIVE_TRAJ_STATUS_INPROGRESS 0
#define AVERSIVE_TRAJ_STATUS_DONE 1
#define AVERSIVE_TRAJ_STATUS_BLOCKED 2


int aversive_open(aversive_dev_t*);
void aversive_close(aversive_dev_t*);
int aversive_is_traj_done(aversive_dev_t*, int*);
int aversive_turn(aversive_dev_t*, int16_t);
int aversive_turn_s(aversive_dev_t*, int16_t);
int aversive_move_forward(aversive_dev_t*, int16_t);
int aversive_move_forward_s(aversive_dev_t*, int16_t);
int aversive_set_speed(aversive_dev_t*, int16_t, int16_t);
int aversive_stop(aversive_dev_t*);
int aversive_get_motors_current(aversive_dev_t*, int16_t*, int16_t*);
int aversive_sync_sequence(aversive_dev_t*);
int aversive_set_asserv(aversive_dev_t*, int16_t);
int aversive_set_power(aversive_dev_t*, int16_t);
int aversive_set_pos(aversive_dev_t*, int16_t, int16_t, int16_t);
int aversive_get_pos(aversive_dev_t*, int16_t*, int16_t*, int16_t*);
int aversive_goto_xy_abs(aversive_dev_t*, int16_t, int16_t);
int aversive_goto_forward_xy_abs(aversive_dev_t*, int16_t, int16_t);
int aversive_traj_unblock(aversive_dev_t*);
int aversive_traj_status(aversive_dev_t*, int*);
int aversive_set_blocking_params(aversive_dev_t*, int16_t, int16_t, int16_t);
int aversive_set_blocking_params2(aversive_dev_t*, int16_t, int16_t);

/* passthru routine */
int aversive_send_msg3(aversive_dev_t*, uint8_t, uint16_t*);

/* debugging routine */
int aversive_read_keyval(aversive_dev_t*, uint16_t, uint16_t*);
int aversive_write_keyval(aversive_dev_t*, uint16_t, uint16_t);


#endif /* ! AVERSIVE_H_INCLUDED */
