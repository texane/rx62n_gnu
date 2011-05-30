#ifndef IGREBOARD_H_INCLUDED
# define IGREBOARD_H_INCLUDED


/* dont forward decl, include to force config */
#include "can_config.h"
#include "can.h"

typedef struct igreboard_dev
{
  can_dev_t* can_dev;
} igreboard_dev_t;


int igreboard_open(igreboard_dev_t*);
void igreboard_close(igreboard_dev_t*);
int igreboard_set_led(igreboard_dev_t*, unsigned int, unsigned int);
int igreboard_open_gripper(igreboard_dev_t*);
int igreboard_close_gripper(igreboard_dev_t*);
int igreboard_get_gripper_switch(igreboard_dev_t*, unsigned int*);
int igreboard_print_string(igreboard_dev_t*, const char*);
int igreboard_ping_device(igreboard_dev_t*);
int igreboard_read_adc(igreboard_dev_t*, unsigned int, unsigned int*);
int igreboard_get_back_switches(igreboard_dev_t*, unsigned int*);
int igreboard_enable_sonar(igreboard_dev_t*);
int igreboard_disable_sonar(igreboard_dev_t*);
int igreboard_read_sonar(igreboard_dev_t*, unsigned int*, unsigned int*);
int igreboard_get_cord_switch(igreboard_dev_t*, unsigned int*);
int igreboard_get_color_switch(igreboard_dev_t*, unsigned int*);



#endif /* ! IGREBOARD_H_INCLUDED */
