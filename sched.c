#include "iodefine.h"
#include "config.h"


void  sched_initialize(void)
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
