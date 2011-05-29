#ifndef SWATCH_H_INCLUDED
# define SWATCH_H_INCLUDED


void swatch_initialize(void);
void swatch_schedule(void);
unsigned int swatch_get_msecs(void);
unsigned int swatch_get_game_msecs(void);
unsigned int swatch_is_game_over(void);
void swatch_start_game(void);


#endif /* ! SWATCH_H_INCLUDED */
