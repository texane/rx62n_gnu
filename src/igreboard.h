#ifndef IGREBOARD_H_INCLUDED
# define IGREBOARD_H_INCLUDED


int igreboard_set_led(unsigned int, unsigned int);
int igreboard_open_gripper(void);
int igreboard_close_gripper(void);
int igreboard_get_gripper_switch(unsigned int*);
int igreboard_print_string(const char*);
int igreboard_ping_device(void);
int igreboard_read_adc(unsigned int, unsigned int*);


#endif /* ! IGREBOARD_H_INCLUDED */
