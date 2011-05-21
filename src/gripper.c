int gripper_initialize(void)
{
  return 0;
}


int gripper_get_switch(unsigned int* is_pushed)
{
  *is_pushed = 0;
  return 0;
}


int gripper_open(void)
{
  return 0;
}


int gripper_close(void)
{
  return 0;
}


int gripper_is_done(unsigned int* is_done)
{
  /* is the last command done */
  *is_done = 0;
  return -1;
}
