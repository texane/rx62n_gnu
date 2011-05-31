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


void fsm_initialize(void);
void fsm_push(fsm_t*);
void fsm_execute_all(void);
void fsm_execute_one(fsm_t*);

/* xxx_fsm factory */
void default_fsm_initialize(fsm_t*);
void takepawn_fsm_initialize(fsm_t*);
void firstpos_fsm_initialize(fsm_t*);
void waitcord_fsm_initialize(fsm_t*);
void emptyarea_fsm_initialize(fsm_t*);
void latpawn_fsm_initialize(fsm_t*);

/* private fsm methods */
unsigned int latpawn_is_pawn(void*);


#endif /* ! FSM_H_INCLUDED */
