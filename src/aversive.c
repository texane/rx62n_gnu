/*
** Made by fabien le mentec <texane@gmail.com>
*/


#include <stdint.h>
#include "iodefine.h"
#include "debug.h"
#include "aversive.h"
#include "config.h"


/* internal */

static inline int send_msg0
(aversive_dev_t* dev, uint8_t cmd, int16_t val)
{
#if CONFIG_USE_I2C

#define I2C_ADDR_AVERSIVE 0x66
  return i2c_send_msg0(&dev->i2c_dev, I2C_ADDR_AVERSIVE, cmd, (uint16_t)val);

#elif CONFIG_USE_CAN

  return can_send_msg0(dev->can_dev, cmd, (uint16_t)val);

#endif
}


static inline int send_msg3(aversive_dev_t* dev, uint8_t cmd, int16_t* values)
{
  /* inout values */

#if CONFIG_USE_I2C

  i2c_send_msg3(&dev->i2c_dev, I2C_ADDR_AVERSIVE, cmd, (uint16_t*)values);
  return 0;

#elif CONFIG_USE_CAN

  return can_send_msg3(dev->can_dev, cmd, (uint16_t*)values);

#endif
}


static inline int send_msg3_0(aversive_dev_t* dev, uint8_t cmd)
{
  /* 0 argument version */

  uint16_t values[3] = {0, 0, 0};

#if CONFIG_USE_I2C
  i2c_send_msg3(&dev->i2c_dev, I2C_ADDR_AVERSIVE, cmd, values);
#elif CONFIG_USE_CAN
  can_send_msg3(dev->can_dev, cmd, values);
#endif

  return 0;
}


static inline int send_msg3_1(aversive_dev_t* dev, uint8_t cmd, int16_t a)
{
  /* 1 argument version */

  uint16_t values[3] = {(uint16_t)a, 0, 0};

#if CONFIG_USE_I2C

  i2c_send_msg3(&dev->i2c_dev, I2C_ADDR_AVERSIVE, cmd, values);
  return 0;

#elif CONFIG_USE_CAN

  return can_send_msg3(dev->can_dev, cmd, values);

#endif
}


/* exported */

int aversive_open(aversive_dev_t* dev)
{
#if CONFIG_USE_I2C

  if (i2c_open(&dev->i2c_dev, 0) == -1)
  {
    DEBUG_ERROR("i2c_open\n");
    return -1;
  }

#elif CONFIG_USE_CAN

  dev->can_dev = can_open();
  if (dev->can_dev == 0)
  {
    DEBUG_ERROR("can_open\n");
    return -1;
  }

#endif /* CONFIG_USE_XXX */

#if (CONFIG_DO_ADC == 0)
#if (CONFIG_DO_KEYVAL == 0)
#if (CONFIG_DO_IGREBOARD == 0)

  /* send the following to initialize aversive
   */

  aversive_sync_sequence(dev);
  aversive_stop(dev);
  aversive_set_asserv(dev, 0);
  aversive_set_power(dev, 0);
  aversive_set_pos(dev, 0, 0, 0);
  aversive_set_power(dev, 1);
  aversive_set_asserv(dev, 1);
  aversive_set_blocking_params(dev, 5, 300, 8000);
  aversive_set_blocking_params2(dev, 150, 150);

#endif /* CONFIG_DO_IGREBOARD == 0 */
#endif /* CONFIG_DO_KEYVAL == 0 */
#endif /* CONFIG_DO_ADC == 0 */

  return 0;
}


void aversive_close(aversive_dev_t* dev)
{
#if CONFIG_USE_I2C
  i2c_close(&dev->i2c_dev);
#elif CONFIG_USE_CAN
  can_close(dev->can_dev);
#endif
}


int aversive_is_traj_done(aversive_dev_t* dev, int* is_done)
{
  *is_done = 0;

#if CONFIG_USE_I2C
  if (i2c_send_msg0(&dev->i2c_dev, I2C_ADDR_AVERSIVE, 'f', 0))
 #elif CONFIG_USE_CAN
  if (can_send_msg0(dev->can_dev, 'f', 0))
#endif
    *is_done = 1;

  return 0;
}


int aversive_turn(aversive_dev_t* dev, int16_t a)
{
  return send_msg3_1(dev, 'T', a);
}


int aversive_turn_s(aversive_dev_t* dev, int16_t a)
{
  return send_msg3_1(dev, 'U', a);
}


int aversive_move_forward(aversive_dev_t* dev, int16_t d)
{
  return send_msg3_1(dev, 'A', d);
}


int aversive_move_forward_s(aversive_dev_t* dev, int16_t d)
{
  return send_msg0(dev, 'b', d);
}


int aversive_set_asserv(aversive_dev_t* dev, int16_t is_on)
{
  return send_msg0(dev, 'r', is_on);
}


int aversive_set_power(aversive_dev_t* dev, int16_t is_on)
{
  return send_msg0(dev, 'p', is_on);
}


int aversive_set_speed(aversive_dev_t* dev, int16_t d, int16_t a)
{
  const int16_t da = (d << 8) | (a & 0xff);
  return send_msg0(dev, 'v', da);
}


int aversive_stop(aversive_dev_t* dev)
{
  return send_msg0(dev, 's', 0);
}


int aversive_get_pos(aversive_dev_t* dev, int16_t* a, int16_t* x, int16_t* y)
{
  int16_t values[3] = {0, 0, 0};

  send_msg3(dev, 'W', values);

  *a = values[0];
  *x = values[1];
  *y = values[2];

  return 0;
}


int aversive_goto_xy_abs(aversive_dev_t* dev, int16_t x, int16_t y)
{
  int16_t values[3] = {x, y, 0};
  return send_msg3(dev, 'K', values);
}


int aversive_goto_forward_xy_abs(aversive_dev_t* dev, int16_t x, int16_t y)
{
  int16_t values[3] = {x, y, 0};
  return send_msg3(dev, 'F', values);
}


int aversive_set_pos(aversive_dev_t* dev, int16_t a, int16_t x, int16_t y)
{
  int16_t values[3] = {a, x, y};
  return send_msg3(dev, 'P', values);
}


int aversive_set_blocking_params(aversive_dev_t* dev, int16_t a, int16_t b, int16_t c)
{
  int16_t values[3] = {a, b, c};
  return send_msg3(dev, 'S', values);
}


int aversive_set_blocking_params2(aversive_dev_t* dev, int16_t a, int16_t b)
{
  int16_t values[3] = {a, b, 0};
  return send_msg3(dev, 'J', values);
}


int aversive_traj_unblock(aversive_dev_t* dev)
{
  return send_msg0(dev, 'l', 1);
}


int aversive_traj_status(aversive_dev_t* dev, int* status)
{
  *status = send_msg0(dev, 'k', 0);
  return 0;
}


int aversive_get_motors_current(aversive_dev_t* dev, int16_t* r, int16_t* l)
{
  int16_t values[3] = {0, 0, 0};

  send_msg3(dev, 'I', values);

  *r = values[0];
  *l = values[1];

  return 0;
}


int aversive_sync_sequence(aversive_dev_t* dev)
{
  send_msg3_0(dev, 'Z');
  return 0;
}


#if 0 /* todo */

int aversive_linear_distance(aversive_dev_t* dev, int16_t* d)
{
  return (int16_t)(10.0 * traj_get_distance(&robot.traj));
}

set_power(int16_t v)
{
  if (v)
    robot.cs_events |=   DO_POWER;
  else
    robot.cs_events &= ~ DO_POWER;
}
	
void set_asserv(short v)
{
  if (v)
    robot.cs_events |=   DO_CS;
  else
    robot.cs_events &= ~ DO_CS;
}

void turnto_abs(short x, short y)
{
  trajectory_turnto_xy(&robot.traj, ((double) x) / 10.0, ((double) y / 10.0));
}

void goto_abs(short x, short y)
{
  trajectory_goto_xy_abs(&robot.traj, ((double) x) / 10.0, ((double) y / 10.0));
}
	
void set_gains_d(short p, short i, short d)
{
  pid_set_gains(&robot.pid_d, p, i, d);
}

void set_gains_a(short p, short i, short d)
{
  pid_set_gains(&robot.pid_a, p, i, d);
}

/* r and l are % */
void set_pwm(char r, char l)
{
  /* Remember kids : PWM 100% is acheived at P1TPER x 2 */
  pwm_igrebot_set(RIGHT_PWM, ((long) P1TPER) * r / 50);
  pwm_igrebot_set(LEFT_PWM, ((long) P1TPER) * l / 50);
}


void get_codeurs(short *r, short *l)
{
  *r = encoders_igrebot_get_value(RIGHT_ENCODER_EXT);
  *l = encoders_igrebot_get_value(LEFT_ENCODER_EXT);
}


#endif /* todo */


/* debugging routines */

int aversive_read_keyval(aversive_dev_t* dev, uint16_t key, uint16_t* val)
{
#if CONFIG_USE_CAN

  /* read write key,val protocol. at most 8 keys. */

  uint16_t vals[] = { key, 0x00, 0x00 };

  if (can_send_msg3(dev->can_dev, 'r', vals))
    return -1;

  *val = vals[1];

  return 0;

#else

  return -1;

#endif
}

int aversive_write_keyval(aversive_dev_t* dev, uint16_t key, uint16_t val)
{
#if CONFIG_USE_CAN

  uint16_t vals[] = { key, val, 0x00 };

  if (can_send_msg3(dev->can_dev, 'w', vals))
    return -1;

  return 0;

#else

  return -1;

#endif
}

int aversive_send_msg3(aversive_dev_t* dev, uint8_t cmd, uint16_t* values)
{
  return send_msg3(dev, cmd, (int16_t*)values);
}
