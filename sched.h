
#ifndef CLOCK_H
#define CLOCK_H

void tick_isr(void) __attribute__ ((interrupt));
void  tick_start (void);

#endif
