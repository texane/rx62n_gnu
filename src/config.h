#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED


/* global static configuration */

#define CONFIG_CPU_FREQ 96000000 /* in hertz */
#define CONFIG_PCLK_FREQ 48000000 /* in hertz */
#define CONFIG_TIMER_FREQ 10 /* in hertz */

#if 0
#define CONFIG_SWATCH_MSECS 90000 /* match duration */
#else
#define CONFIG_SWATCH_MSECS 100000 /* match duration */
#endif

#define CONFIG_DO_SQUARE 0
#define CONFIG_DO_KEYVAL 0
#define CONFIG_DO_TAKEPAWN 0
#define CONFIG_DO_IGREBOARD 1

#define CONFIG_ENABLE_AVERSIVE 1
#define CONFIG_ENABLE_IGREBOARD 1
#define CONFIG_ENABLE_SHARP 0
#define CONFIG_ENABLE_SWITCHES 0
#define CONFIG_ENABLE_RADAR 0
#define CONFIG_ENABLE_BLINKER 0
#define CONFIG_ENABLE_SWATCH 1
#define CONFIG_ENABLE_GRIPPER 1


#endif /* CONFIG_H_INCLUDED  */
