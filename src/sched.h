#ifndef SCHED_H_INCLUDED
# define SCHED_H_INCLUDED

void tick_isr(void) __attribute__((interrupt));
void  sched_initialize(void);

#endif /* SCHED_H_INCLUDED */
