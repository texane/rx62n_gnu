#include "config.h"
#include <stdint.h>
#include "iodefine.h"


/* internal */

/* this is still messy since aversive_dev_t is
   required to send a can message... this will
   be removed in a near futur, and we mask this
   dependency with the send_recv_msg routine.
 */

#define IGREBOARD_CMD_SET_LED 0x00
#define IGREBOARD_CMD_OPEN_GRIPPER 0x01
#define IGREBOARD_CMD_CLOSE_GRIPPER 0x02
#define IGREBOARD_CMD_GET_GRIPPER_SWITCH 0x03
#define IGREBOARD_CMD_PRINT_BUF 0x04
#define IGREBOARD_CMD_PING_DEVICE 0x05
#define IGREBOARD_CMD_READ_ADC 0x06
#define IGREBOARD_CMD_UNKNOWN ((uint8_t)-1)


#include "aversive.h"

extern aversive_dev_t aversive_device;

static inline int send_recv_msg(uint8_t cmd, uint16_t* values)
{
  return aversive_send_msg3(&aversive_device, cmd, values);
}


/* exported */

int igreboard_set_led(unsigned int index, unsigned int value)
{
  uint16_t values[] = { index, value, 0 };
  return send_recv_msg(IGREBOARD_CMD_SET_LED, values);
}

int igreboard_open_gripper(void)
{
  uint16_t values[] = { 0, 0, 0 };
  return send_recv_msg(IGREBOARD_CMD_OPEN_GRIPPER, values);
}

int igreboard_close_gripper(void)
{
  uint16_t values[] = { 0, 0, 0 };
  return send_recv_msg(IGREBOARD_CMD_CLOSE_GRIPPER, values);
}

int igreboard_get_gripper_switch(unsigned int* value)
{
  uint16_t values[] = { 0, 0, 0 };
  if (send_recv_msg(IGREBOARD_CMD_GET_GRIPPER_SWITCH, values))
    return -1;
  *value = (unsigned int)values[0];
  return 0;
}

int igreboard_print_string(const char* s)
{
  return send_recv_msg(IGREBOARD_CMD_PRINT_BUF, (uint16_t*)s);
}

int igreboard_ping_device(void)
{
  uint16_t values[] = { 0, 0, 0 };
  return send_recv_msg(IGREBOARD_CMD_PING_DEVICE, values);
}


int igreboard_read_adc(unsigned int chan, unsigned int* value)
{
  uint16_t values[] = { (uint16_t)chan, 0, 0 };
  if (send_recv_msg(IGREBOARD_CMD_READ_ADC, values))
    return -1;
  *value = (unsigned int)values[1];
  return 0;
}
