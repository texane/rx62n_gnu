#include "config.h"
#include <stdint.h>
#include "iodefine.h"
#include "can_config.h"
#include "can.h"
#include "igreboard.h"


#define IGREBOARD_CMD_SET_LED 0x00
#define IGREBOARD_CMD_OPEN_GRIPPER 0x01
#define IGREBOARD_CMD_CLOSE_GRIPPER 0x02
#define IGREBOARD_CMD_GET_GRIPPER_SWITCH 0x03
#define IGREBOARD_CMD_PRINT_BUF 0x04
#define IGREBOARD_CMD_PING_DEVICE 0x05
#define IGREBOARD_CMD_READ_ADC 0x06
#define IGREBOARD_CMD_GET_BACK_SWITCHES 0x07
#define IGREBOARD_CMD_ENABLE_SONAR 0x08
#define IGREBOARD_CMD_READ_SONAR 0x09
#define IGREBOARD_CMD_UNKNOWN ((uint8_t)-1)


static inline int send_recv_msg
(igreboard_dev_t* dev, uint8_t cmd, uint16_t* values)
{
  return can_send_msg3(dev->can_dev, cmd, values);
}


/* exported */

int igreboard_open(igreboard_dev_t* dev)
{
  dev->can_dev = can_open();
  if (dev->can_dev == 0)
    return -1;
  return 0;
}

void igreboard_close(igreboard_dev_t* dev)
{
  can_close(dev->can_dev);
}

int igreboard_set_led
(igreboard_dev_t* dev, unsigned int index, unsigned int value)
{
  uint16_t values[] = { index, value, 0 };
  return send_recv_msg(dev, IGREBOARD_CMD_SET_LED, values);
}

int igreboard_open_gripper(igreboard_dev_t* dev)
{
  uint16_t values[] = { 0, 0, 0 };
  return send_recv_msg(dev, IGREBOARD_CMD_OPEN_GRIPPER, values);
}

int igreboard_close_gripper(igreboard_dev_t* dev)
{
  uint16_t values[] = { 0, 0, 0 };
  return send_recv_msg(dev, IGREBOARD_CMD_CLOSE_GRIPPER, values);
}

int igreboard_get_gripper_switch
(igreboard_dev_t* dev, unsigned int* value)
{
  uint16_t values[] = { 0, 0, 0 };
  if (send_recv_msg(dev, IGREBOARD_CMD_GET_GRIPPER_SWITCH, values))
    return -1;
  *value = (unsigned int)values[0];
  return 0;
}

int igreboard_print_string(igreboard_dev_t* dev, const char* s)
{
  return send_recv_msg(dev, IGREBOARD_CMD_PRINT_BUF, (uint16_t*)s);
}

int igreboard_ping_device(igreboard_dev_t* dev)
{
  uint16_t values[] = { 0, 0, 0 };
  return send_recv_msg(dev, IGREBOARD_CMD_PING_DEVICE, values);
}

int igreboard_read_adc
(igreboard_dev_t* dev, unsigned int chan, unsigned int* value)
{
  uint16_t values[] = { (uint16_t)chan, 0, 0 };
  if (send_recv_msg(dev, IGREBOARD_CMD_READ_ADC, values))
    return -1;
  *value = (unsigned int)values[1];
  return 0;
}

int igreboard_get_back_switches(igreboard_dev_t* dev, unsigned int* map)
{
  uint16_t values[] = { 0, 0, 0 };
  if (send_recv_msg(dev, IGREBOARD_CMD_GET_BACK_SWITCHES, values))
    return -1;
  *map = (unsigned int)values[0];
  return 0;
}

int igreboard_enable_sonar(igreboard_dev_t* dev)
{
  uint16_t values[] = { 1, 0, 0 };
  if (send_recv_msg(dev, IGREBOARD_CMD_ENABLE_SONAR, values))
    return -1;
  return 0;
}

int igreboard_disable_sonar(igreboard_dev_t* dev)
{
  uint16_t values[] = { 0, 0, 0 };
  if (send_recv_msg(dev, IGREBOARD_CMD_ENABLE_SONAR, values))
    return -1;
  return 0;
}

int igreboard_read_sonar
(igreboard_dev_t* dev, unsigned int* a, unsigned int* d)
{
  uint16_t values[] = { 0, 0, 0 };
  if (send_recv_msg(dev, IGREBOARD_CMD_READ_SONAR, values))
    return -1;
  *a = (unsigned int)values[0];
  *d = (unsigned int)values[1];
  return 0;
}
