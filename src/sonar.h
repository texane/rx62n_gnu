#ifndef SONAR_H_INCLUDED
# define SONAR_H_INCLUDED


#include <stdint.h>


void sonar_initialize(void);
void sonar_finalize(void);
unsigned int sonar_is_detected(void);
void sonar_schedule(void);


#endif /* ! SONAR_H_INCLUDED */
