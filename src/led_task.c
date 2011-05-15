#include "iodefine.h"
#include "yrdkrx62ndef.h"


void led_task_initialize(void)
{
  LED4_DDR = 1;
}

void led_task_schedule(void)
{
  static unsigned int xor = 0;
  LED4 = xor;
  xor ^= 1;
}
