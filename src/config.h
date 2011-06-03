#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED


/* global static configuration */

#define CONFIG_CPU_FREQ 96000000 /* in hertz */
#define CONFIG_PCLK_FREQ 48000000 /* in hertz */
#define CONFIG_TIMER_FREQ 10 /* in hertz */

#define CONFIG_SWATCH_MSECS 85000 /* match duration */

#define CONFIG_UNIT_SQUARE 0
#define CONFIG_UNIT_KEYVAL 0
#define CONFIG_UNIT_LATPAWN 0
#define CONFIG_UNIT_TAKEPAWN 0
#define CONFIG_UNIT_FIRSTPOS 0
#define CONFIG_UNIT_IGREBOARD 0
#define CONFIG_UNIT_AVERSIVE 0
#define CONFIG_UNIT_ADC 0
#define CONFIG_UNIT_SONAR 0
#define CONFIG_UNIT_MATCH 0
#define CONFIG_UNIT_SENSOR 0
#define CONFIG_UNIT_POS 0
#define CONFIG_UNIT_HOMOL 1
#define CONFIG_UNIT_ROTATE 0
#define CONFIG_UNIT_PUTPAWN 0
#define CONFIG_UNIT_CENTER 0
#define CONFIG_UNIT_BONUS 0

#define CONFIG_ENABLE_AVERSIVE 1
#define CONFIG_ENABLE_IGREBOARD 1
#define CONFIG_ENABLE_SHARP 1
#define CONFIG_ENABLE_SWITCHES 0
#define CONFIG_ENABLE_SONAR 1
#define CONFIG_ENABLE_BLINKER 0
#define CONFIG_ENABLE_SWATCH 1

#if (CONFIG_ENABLE_SONAR == 0)
# warning TOREMOVE_FOR_MATCH
#endif

#if (CONFIG_UNIT_HOMOL == 0)
# warning TOREMOVE_FOR_MATCH
#endif

#endif /* CONFIG_H_INCLUDED  */
