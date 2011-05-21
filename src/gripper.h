#ifndef GRIPPER_H_INCLUDED
# define GRIPPER_H_INCLUDED


int gripper_initialize(void);
int gripper_get_switch(unsigned int*);
int gripper_open(void);
int gripper_close(void);
int gripper_is_done(unsigned int*);


#endif /* ! GRIPPER_H_INCLUDED */
