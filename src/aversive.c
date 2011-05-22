/*
** Made by fabien le mentec <texane@gmail.com>
*/


#include <stdint.h>
#include "iodefine.h"
#include "debug.h"
#include "lcd.h"
#include "aversive.h"
#include "config.h"


#if CONFIG_USE_CAN


/* CAN low level routines. i2c equivalent here:
   trunk\Info\2010\sbc2410\i2c-testing\master
 */

#if 0 /* kpit gnu compiler uses single byte packing by default */
# define ATTRIBUTE_PACKED
#else
# define ATTRIBUTE_PACKED __attribute__((packed))
#endif


struct can_msg0
{
  uint8_t cmd;
  uint8_t value[2];
} ATTRIBUTE_PACKED;

struct can_msg3
{
  uint8_t cmd;
  uint8_t seq;
  uint16_t value[3];
} ATTRIBUTE_PACKED;


/* todo: use static when no more external dependencies */

/* interrupt related flags */
#ifndef USE_CAN_POLL
uint32_t can0_tx_sentdata_flag = 0;
uint32_t can0_tx_remote_sentdata_flag = 0;
uint32_t can0_rx_newdata_flag = 0;
uint32_t can0_rx_test_newdata_flag = 0;
uint32_t can0_rx_remote_frame_flag = 0;
#endif /* USE_CAN_POLL */

/* frames */
can_std_frame_t tx_dataframe;
can_std_frame_t rx_dataframe;
can_std_frame_t remote_frame;

/* node CAN state */
static uint32_t error_bus_status;
static uint32_t error_bus_status_prev;


/* CAN io wrappers */

#if 1 /* LITTLE_ENDIAN */

# define mach_to_le(__val) __val
# define le_to_mach(__val) __val

#else

static inline uint16_t mach_to_le(uint16_t val)
{ return (value & 0xff) << 8 | ((value & 0xff00) >> 8); }

static inline uint16_t le_to_mach(uint16_t val)
{ return mach_to_le(val); }

#endif

static int send_frame(aversive_dev_t* dev)
{
  dev->tx_dataframe->id = dev->sid;
  dev->tx_dataframe->dlc = 8;

  if (R_CAN_TxSet(0, CANBOX_TX, dev->tx_dataframe, DATA_FRAME) != R_CAN_OK)
    return -1;

#ifdef USE_CAN_POLL

#if 1 /* NEEDED */

  /* the documentation suggests calling this function can be
     omitted since we can reasonably assume the message has
     been sent on TxSet success. BUT not checking it, we
     possibly destroy the frame contents before it gets sent
   */

  while (R_CAN_TxCheck(0, CANBOX_TX))
    ;

#endif /* TODO */

#else /* can interrupts */

  while (can0_tx_sentdata_flag == 0)
  {
    aversive_poll_bus(dev);
  }

  can0_tx_sentdata_flag = 0;

#endif /* USE_CAN_POLL */

  return 0;
}


static int recv_frame(aversive_dev_t* dev)
{
#ifdef USE_CAN_POLL

  /* assuming 100mhz, wait for approx N x 0.1 secs max */
  unsigned int count = 2000000;

  while (R_CAN_RxPoll(0, CANBOX_RX) != R_CAN_OK)
  {
    if (--count == 0)
      return -1;
  }

  if (R_CAN_RxRead(0, CANBOX_RX, dev->rx_dataframe) != R_CAN_OK)
    return -1;

  return 0;

#else /* can interrupts */

  if (can0_rx_newdata_flag)
  {
    can0_rx_newdata_flag = 0;
	
    if (R_CAN_RxRead(0, CANBOX_RX, dev->rx_dataframe) != R_CAN_OK)
    {
      can0_rx_newdata_flag = 0;
      return -1;
    }
  }

  if (can0_rx_test_newdata_flag)
  {
    can0_rx_test_newdata_flag = 0;
  }

  /* Set up remote reply if remote request came in. */
  if (can0_rx_remote_frame_flag == 1)
  {
    /* should not occur */
    can0_rx_remote_frame_flag = 0;
    R_CAN_TxSet(0, CANBOX_REMOTE_TX, dev->remote_frame, DATA_FRAME);
  }

#endif /* USE_CAN_POLL */
}


uint16_t can_send_msg0(aversive_dev_t* dev, uint8_t cmd, uint16_t value)
{
#define CONFIG_SEND_TRIALS 5
  unsigned int trials = CONFIG_SEND_TRIALS;

  struct can_msg0* msg;

  for (; trials; --trials)
  {
    msg = (struct can_msg0*)dev->tx_dataframe->data;

    msg->cmd = cmd;
    msg->value[0] = (value & 0xff00) >> 8;
    msg->value[1] = value & 0xff;

    if (send_frame(dev) == -1)
      continue ;

    msg = (struct can_msg0*)dev->rx_dataframe->data;

    /* reset command for further check */
    msg->cmd = 0;

    if (recv_frame(dev) == -1)
      continue ;

    /* not the intended message, redo */
    if (msg->cmd != cmd)
      continue ;

    break;
  }

  return le_to_mach(*(uint16_t*)msg->value);
}


int can_send_msg3(aversive_dev_t* dev, uint8_t cmd, uint16_t* values)
{
  /* long version */

  unsigned int trials = CONFIG_SEND_TRIALS;
  uint8_t seq;
  struct can_msg3* msg;

  /* pick a sequence number */
  seq = ++dev->seq;

  for (; trials; --trials)
  {
    msg = (struct can_msg3*)dev->tx_dataframe->data;

    msg->cmd = cmd;
    msg->value[0] = mach_to_le(values[0]);
    msg->value[1] = mach_to_le(values[1]);
    msg->value[2] = mach_to_le(values[2]);
    msg->seq = seq;

    if (send_frame(dev) == -1)
      continue ;

    msg = (struct can_msg3*)dev->rx_dataframe->data;

    /* reset command for further check */
    msg->cmd = 0;

    if (recv_frame(dev) == -1)
      continue ;

    if (msg->cmd != cmd)
      continue ;

    break;
  }

  if (trials == 0)
    return -1;

  values[0] = msg->value[0];
  values[1] = msg->value[1];
  values[2] = msg->value[2];

  return 0;
}

#endif /* CONFIG_USE_CAN, low level routines */


/* internal */

static inline int send_msg0
(aversive_dev_t* dev, uint8_t cmd, int16_t val)
{
#if CONFIG_USE_I2C

#define I2C_ADDR_AVERSIVE 0x66
  return i2c_send_msg0(&dev->i2c_dev, I2C_ADDR_AVERSIVE, cmd, (uint16_t)val);

#elif CONFIG_USE_CAN

  return can_send_msg0(dev, cmd, (uint16_t)val);

#endif
}


static inline int send_msg3(aversive_dev_t* dev, uint8_t cmd, int16_t* values)
{
  /* inout values */

#if CONFIG_USE_I2C

  i2c_send_msg3(&dev->i2c_dev, I2C_ADDR_AVERSIVE, cmd, (uint16_t*)values);
  return 0;

#elif CONFIG_USE_CAN

  return can_send_msg3(dev, cmd, (uint16_t*)values);

#endif
}


static inline int send_msg3_0(aversive_dev_t* dev, uint8_t cmd)
{
  /* 0 argument version */

  uint16_t values[3] = {0, 0, 0};

#if CONFIG_USE_I2C
  i2c_send_msg3(&dev->i2c_dev, I2C_ADDR_AVERSIVE, cmd, values);
#elif CONFIG_USE_CAN
  can_send_msg3(dev, cmd, values);
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

  return can_send_msg3(dev, cmd, values);

#endif
}


#if CONFIG_USE_CAN

static int setup_can_hardware(aversive_dev_t* dev)
{
  /* note: assume HardwareSetup called
   */

  /* Configure mailboxes in Halt mode. */
  if (R_CAN_Control(0, HALT_CANMODE) != R_CAN_OK)
  {
    lcd_string(4, 0, "r_can_control");
    return -1;
  }

  /* reset bus states */
  error_bus_status = 0;
  error_bus_status_prev = 0;

  /* reset error judge factor and error code registers */
  CAN0.EIFR.BYTE = 0;
  CAN0.ECSR.BYTE = 0;

  /* reset error counters */
  CAN0.RECR = 0;
  CAN0.TECR = 0;

  /* setup receiving mailboxes */
  if (R_CAN_RxSet(0, CANBOX_RX, 0, DATA_FRAME) != R_CAN_OK)
  {
    lcd_string(4, 0, "r_can_rxset_data");
    return -1;
  }

  if (R_CAN_RxSet(0, CANBOX_REMOTE_RX, 0, REMOTE_FRAME) != R_CAN_OK)
  {
    lcd_string(4, 0, "r_can_rxset_remote");
    return -1;
  }

  /* receivall mailboxes (mask == 0), ie.:
     if ((frame->sid & mbox->mask) == mbox.id) mbox.data = frame.data
     note that R_CAN_RxSetMask enables CANMODE, so we call those after RxSet.
   */
  R_CAN_RxSetMask(0, CANBOX_RX, 0);
  R_CAN_RxSetMask(0, CANBOX_REMOTE_RX, 0);
		
  /* switch to operate mode. */
  if (R_CAN_Control(0, OPERATE_CANMODE) != R_CAN_OK)
  {
    lcd_string(4, 0, "r_can_control");
    return -1;
  }

  return 0;
}

#endif /* CONFIG_USE_CAN */


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

  /* initialize CAN hardware
   */

  if (R_CAN_Create(0) != R_CAN_OK)
  {
    lcd_string(4, 0, "r_can_create");
    return -1;
  }

  if (R_CAN_PortSet(0, ENABLE) != R_CAN_OK)
  {
    lcd_string(4, 0, "r_can_portset");
    return -1;
  }

  if (setup_can_hardware(dev) == -1)
  {
    return -1;
  }

  /* assign internal fields
   */

  dev->sid = 0x123;
  dev->seq = 0;

  dev->tx_dataframe = &tx_dataframe;
  tx_dataframe.id = dev->sid;
  tx_dataframe.dlc = 8;

  dev->rx_dataframe = &rx_dataframe;
  rx_dataframe.id = 0;

  dev->remote_frame = &remote_frame;
  remote_frame.id = 0;

#endif

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

  return 0;
}


void aversive_close(aversive_dev_t* dev)
{
#if CONFIG_USE_I2C
  i2c_close(&dev->i2c_dev);
#endif
}


#if CONFIG_USE_CAN

int aversive_poll_bus(aversive_dev_t* dev)
{
  can_std_frame_t err_tx_dataframe;
	
  error_bus_status = R_CAN_CheckErr(0);

  /* state has changed */
  if (error_bus_status == error_bus_status_prev)
    return 0;

  switch (error_bus_status)
  {
    /* ACTIVE state */
  case R_CAN_STATUS_ERROR_ACTIVE:
    {
      /* restart from BUSOFF state */
      if (error_bus_status_prev == R_CAN_STATUS_BUSOFF)
      {
	if (setup_can_hardware(dev) == -1)
	  return -1;
      }
      break;
    }

    /* PASSIVE state */
  case R_CAN_STATUS_ERROR_PASSIVE:
    {
      break ;
    }

    /* BUSOFF state */
  case R_CAN_STATUS_BUSOFF:
    {
      break ;
    }

  default:
    {
      break ;
    }
  } /* switch state */

  /* update previous state */
  error_bus_status_prev = error_bus_status;

  /* transmit CAN bus status change */
  err_tx_dataframe.id = 0x700;
  err_tx_dataframe.dlc = 1;
  err_tx_dataframe.data[0] = error_bus_status;
  R_CAN_TxSet(0, CANBOX_ERROR_TX, &err_tx_dataframe, DATA_FRAME);

  return 0;
}

#endif /* aversive_poll_bus */


int aversive_is_traj_done(aversive_dev_t* dev, int* is_done)
{
  *is_done = 0;

#if CONFIG_USE_I2C
  if (i2c_send_msg0(&dev->i2c_dev, I2C_ADDR_AVERSIVE, 'f', 0))
 #elif CONFIG_USE_CAN
  if (can_send_msg0(dev, 'f', 0))
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
  /* read write key,val protocol. at most 8 keys. */

  uint16_t vals[] = { key, 0x00, 0x00 };

  if (can_send_msg3(dev, 'r', vals))
    return -1;

  *val = vals[1];

  return 0;
}

int aversive_write_keyval(aversive_dev_t* dev, uint16_t key, uint16_t val)
{
  uint16_t vals[] = { key, val, 0x00 };

  if (can_send_msg3(dev, 'w', vals))
    return -1;

  return 0;
}

int aversive_read(aversive_dev_t* dev, uint8_t* buf)
{
  /* blocking function */

  unsigned int i;

  if (recv_frame(dev) == -1)
    return -1;

  for (i = 0; i < 8; ++i)
    buf[i] = dev->rx_dataframe->data[i];

  return 0;
}


int aversive_send_msg3(aversive_dev_t* dev, uint8_t cmd, uint16_t* values)
{
  return send_msg3(dev, cmd, (int16_t*)values);
}
