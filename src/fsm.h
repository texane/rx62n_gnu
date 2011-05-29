#ifndef FSM_H_INCLUDED
# define FSM_H_INCLUDED


typedef struct fsm
{
  /* methods */
  void (*next)(void*);
  unsigned int (*is_done)(void*);
  void (*preempt)(void*);
  void (*restart)(void*);

  /* private data */
  void* data;

} fsm_t;


/* fsm scheduler */
void fsm_main(fsm_t*);

/* xxx_fsm factory */
void takepawn_fsm_initialize(fsm_t*);


#endif /* ! FSM_H_INCLUDED */
