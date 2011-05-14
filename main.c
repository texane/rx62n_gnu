#include "iodefine.h"
#include "yrdkrx62ndef.h"
#include "lcd.h"
#include "clock.h"

int flag = 0;
int i = 1;
int j = 1;

void led_cycle()
{
      if ((i == j) && !flag)
	{
	  i = i << 1;
	  if (! (i & 0xfff))
	    i = 1;
	  flag = 1;
	}

      j = j << 1;
      if (! (j & 0xfff))
	j = 1;

      if (i != j)
	flag = 0;

#define b(led,bit) led = ((j|i) & (1<<bit)) ? 0 : 1
      b(LED4,0);
      b(LED5,1);
      b(LED6,2);
      b(LED7,3);
      b(LED8,4);
      b(LED9,5);
      b(LED10,6);
      b(LED11,7);
      b(LED12,8);
      b(LED13,9);
      b(LED14,10);
      b(LED15,11);
}

void tick_isr(void) 
{
    led_cycle();
}

int main()
{
  LED4_DDR = 1;
  LED5_DDR = 1;
  LED6_DDR = 1;
  LED7_DDR = 1;
  LED8_DDR = 1;
  LED9_DDR = 1;
  LED10_DDR = 1;
  LED11_DDR = 1;
  LED12_DDR = 1;
  LED13_DDR = 1;
  LED14_DDR = 1;
  LED15_DDR = 1;

  lcd_open();
  lcd_string(2,0, "Hello");
  lcd_string(3,0, "Freedom");
  lcd_string(4,0, "lovers!");
 
  tick_start(); // start CMT which will call tick_isr() upon interrupt

  while (1)
  {
      asm("wait");
  }
}
