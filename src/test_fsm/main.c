#include "./config.h"
#include <stdio.h>
#include <stdlib.h>
#include "../fsm.h"


typedef struct print_fsm
{
  unsigned int id;
  unsigned int pass;
} print_fsm_t;

static unsigned int print_fsm_isdone(void* data_)
{
  print_fsm_t* const data = data_;
  return data->pass == 4;
}

static void print_fsm_preempt(void* data)
{
}

static void print_fsm_restart(void* data_)
{
}

void print_fsm_initialize(fsm_t*);
static void print_fsm_next(void* data_)
{
  print_fsm_t* const data = data_;
  printf("[%c] %u\n", 'a' + data->id, data->pass++);

/*   if ((data->pass) == 1 && (data->id <= 4)) */
/*   { */
/*     fsm_t* const fsm = malloc(sizeof(fsm_t)); */
/*     print_fsm_initialize(fsm); */
/*     fsm_push(fsm); */
/*   } */
}

void print_fsm_initialize(fsm_t* fsm)
{
  print_fsm_t* const data = malloc(sizeof(print_fsm_t));
  static unsigned int id = 0;

  fsm->next = print_fsm_next;
  fsm->is_done = print_fsm_isdone;
  fsm->preempt = print_fsm_preempt;
  fsm->restart = print_fsm_restart;

  data->id = id++;
  data->pass = 0;
  fsm->data = data;
}


int main(int ac, char** av)
{
  fsm_t a, b;

  print_fsm_initialize(&a);
  fsm_push(&a);
  fsm_execute_one(&a);

/*   print_fsm_initialize(&b); */
/*   fsm_push(&b); */

/*   fsm_execute_all(); */

/*   print_fsm_initialize(&a); */
/*   fsm_push(&a); */
/*   fsm_execute_all(); */

  return 0;
}
