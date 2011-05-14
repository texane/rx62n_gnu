#include "iodefine.h"
#include "config.h"

void set_system_clock_speed (void)
{
  /* We start with a 12.5 MHz crystal.  */

  /* 0 = x8  1 = x4  2 = x2  3 = x1 */
  /* ICLK: x8 = 100 MHz  */
  /* PCLK: x4 =  50 MHz   */
  /* BCLK: x2 =  25 MHz   */

  PORT5.DDR.BIT.B5 = 0; /* BCLK */
  /*          B = 8 if BCLK */
  /*         I B P	*/
  SYSTEM.SCKCR.LONG = 0x01810100;
  /*	     0 = 100 MHz
	     1 =  50 MHz
	     2 =  25 MHz
	     3 =  12 MHz
  */
  PORT5.DDR.BIT.B5 = 1; /* BCLK */
  SYSTEM.SCKCR.LONG &= ~0x00800000;
}

void  tick_start (void)
{
    // install the interrupt vector table
    asm("mvtc #_vectors, intb");

    MSTP(CMT0) = 0;           // Enable CMT0 module
    CMT.CMSTR0.BIT.STR0 = 0;  // Stop timer channel 0
    CMT0.CMCR.BIT.CKS = 3; /* PCLK / 512 */
    CMT0.CMCOR = CONFIG_PCLK_FREQ / (512 * CONFIG_TIMER_FREQ);
    CMT0.CMCNT = 0;           // Clear the counter

    IR(CMT0, CMI0)  = 0;      // Clear any pending interrupt
    IPR(CMT0,)      = 3;      // Interrupt priority
    IEN(CMT0, CMI0) = 1;      // Enable CMT0 as an interrupt source
    CMT0.CMCR.BIT.CMIE = 1;   // Enable CMT0 interrupt
    CMT.CMSTR0.BIT.STR0 = 1;  // Start CMT0
}
