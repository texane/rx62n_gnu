#include "iodefine.h"
#include "yrdkrx62ndef.h"


void blinker_initialize(void)
{
  LED4_DDR = 1;
}

void blinker_schedule(void)
{
  static unsigned int xor = 0;
  LED4 = xor;
  xor ^= 1;
}
