#ifndef RADAR_H_INCLUDED
# define RADAR_H_INCLUDED


#include <stdint.h>


void radar_initialize(void);
void radar_enable(void);
void radar_disable(void);
void radar_read_pos(uint16_t*, uint16_t*);
void radar_schedule(void);


#endif /* ! RADAR_H_INCLUDED */
