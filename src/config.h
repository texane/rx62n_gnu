#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED


/* global static configuration */

#define CONFIG_CPU_FREQ 96000000 /* in hertz */
#define CONFIG_PCLK_FREQ 48000000 /* in hertz */
#define CONFIG_TIMER_FREQ 10 /* in hertz */

#define CONFIG_SWATCH_MSECS 5000 /* match duration */

#define CONFIG_DO_SQUARE 0
#define CONFIG_DO_KEYVAL 0
#define CONFIG_DO_PAWN 0

#define CONFIG_ENABLE_AVERSIVE 0
#define CONFIG_ENABLE_SHARP 1
#define CONFIG_ENABLE_SWITCHES 0
#define CONFIG_ENABLE_RADAR 0
#define CONFIG_ENABLE_BLINKER 0
#define CONFIG_ENABLE_SWATCH 0


#endif /* CONFIG_H_INCLUDED  */
