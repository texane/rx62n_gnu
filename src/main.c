#include "config.h"

#include <stdint.h>
#include "iodefine.h"
#include "yrdkrx62ndef.h"
#include "lcd.h"
#include "sched.h"
#include "hwsetup.h"
#include "fsm.h"
#include "unit.h"

#if CONFIG_ENABLE_SWITCHES
# include "switches.h"
#endif

#if CONFIG_ENABLE_SWATCH
# include "swatch.h"
#endif

#if CONFIG_ENABLE_BLINKER
# include "blinker.h"
#endif

#if CONFIG_ENABLE_SONAR
# include "sonar.h"
#endif

#if CONFIG_ENABLE_SHARP
# include "sharp.h"
#endif

#if CONFIG_ENABLE_IGREBOARD
# include "igreboard.h"
igreboard_dev_t igreboard_device;
#endif /* CONFIG_ENABLE_IGREBOARD */

#if CONFIG_ENABLE_AVERSIVE
# include "aversive.h"
aversive_dev_t aversive_device;
#endif /* CONFIG_ENABLE_AVERSIVE */


/* global initialization routines */

static int initialize(void)
{
  /* 100MHz CPU clock, configure ports, peripherals ... */
  HardwareSetup();

  /* init devices */
  lcd_open();

#if CONFIG_ENABLE_AVERSIVE
  if (aversive_open(&aversive_device) == -1)
  {
    lcd_string(3, 0, "[!] aversive_open");
    return -1;
  }
#endif /* CONFIG_ENABLE_AVERSIVE */

#if CONFIG_ENABLE_IGREBOARD
  if (igreboard_open(&igreboard_device) == -1)
  {
    lcd_string(3, 0, "[!] igreboard_open");
    return -1;
  }
#endif /* CONFIG_ENABLE_IGREBOARD */

#if CONFIG_ENABLE_BLINKER
  blinker_initialize();
#endif

#if CONFIG_ENABLE_SHARP
  sharp_initialize_all();
#endif

#if CONFIG_ENABLE_SWITCHES
  switches_initialize();
#endif

#if CONFIG_ENABLE_SWATCH
  swatch_initialize();
#endif

#if CONFIG_ENABLE_SONAR
  /* sonar_initialize(); */
#endif

  /* init and start the scheduler */
  sched_initialize();

  return 0;
}

static void finalize(void)
{
#if CONFIG_ENABLE_SONAR
  sonar_finalize();
#endif

#if CONFIG_ENABLE_AVERSIVE
  aversive_close(&aversive_device);
#endif

#if CONFIG_ENABLE_IGREBOARD
  igreboard_close(&igreboard_device);
#endif
}


/* main */

int main(void)
{
  if (initialize() == -1)
  {
    while (1) asm("wait");
  }

#if CONFIG_UNIT_KEYVAL
  unit_keyval();
#elif CONFIG_UNIT_SQUARE
  unit_square();
#elif CONFIG_UNIT_TAKEPAWN
  unit_takepawn();
#elif CONFIG_UNIT_PUTPAWN
  unit_putpawn();
#elif CONFIG_UNIT_FIRSTPOS
  unit_firstpos();
#elif CONFIG_UNIT_IGREBOARD
  unit_igreboard();
#elif CONFIG_UNIT_AVERSIVE
  unit_aversive();
#elif CONFIG_UNIT_ADC
  unit_adc();
#elif CONFIG_UNIT_SONAR
  unit_sonar();
#elif CONFIG_UNIT_SENSOR
  unit_sensor();
#elif CONFIG_UNIT_LATPAWN
  unit_latpawn();
#elif CONFIG_UNIT_POS
  unit_pos();
#elif CONFIG_UNIT_HOMOL
  unit_homol();
#elif CONFIG_UNIT_ROTATE
  unit_rotate();
#elif CONFIG_UNIT_CENTER
  unit_center();
#else
  while (1) asm("wait\n");
#endif

  finalize();

  return 0;
}


/* dont move interrupt handlers */

void tick_isr(void) 
{
#if CONFIG_ENABLE_SWITCHES
  switches_schedule();
#endif

#if CONFIG_ENABLE_BLINKER
  blinker_schedule();
#endif

#if CONFIG_ENABLE_SWATCH
  swatch_schedule();
#endif

#if 0
#if CONFIG_ENABLE_SONAR
  sonar_schedule();
#endif
#endif
}
