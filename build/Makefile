
CFLAGS = -Wall -O2 -MMD -Wno-strict-aliasing -mint-register=4 -mlittle-endian-data -I../src

PATH := $(PATH):/opt/gnurx/bin/:$(HOME)/install/bin

.PRECIOUS: %.o %.elf

ELF =	main
OBJS =				\
	../src/main.o		\
	../src/hwsetup.o	\
	../src/sched.o		\
	../src/tile.o		\
	../src/fsm.o		\
	../src/default_fsm.o	\
	../src/detected_fsm.o	\
	../src/takepawn_fsm.o	\
	../src/latpawn_fsm.o	\
	../src/firstpos_fsm.o	\
	../src/waitcord_fsm.o	\
	../src/emptyarea_fsm.o	\
	../src/leavestart_fsm.o	\
	../src/switches.o	\
	../src/blinker.o	\
	../src/swatch.o		\
	../src/sonar.o		\
	../src/adc.o		\
	../src/sharp.o		\
	../src/aversive.o	\
	../src/igreboard.o	\
	../src/can.o		\
	../src/lcd.o		\
	../src/font_x5x7.o	\
	../src/unit_keyval.o	\
	../src/unit_square.o	\
	../src/unit_takepawn.o	\
	../src/unit_firstpos.o	\
	../src/unit_igreboard.o	\
	../src/unit_aversive.o	\
	../src/unit_adc.o	\
	../src/unit_sonar.o	\
	../src/unit_sensor.o	\
	../src/unit_latpawn.o	\
	../src/unit_pos.o	\
	../src/unit_rotate.o	\
	../src/unit_homol.o	\
	../src/unit_center.o	\
	../src/unit_bonus.o	\
	../src/interrupt_table.o

all : $(ELF).elf

flash : $(ELF).elf
	rxusb -v $(ELF).elf

%.elf : ../src/crt0.o $(OBJS) rx62n.ld Makefile
	rx-elf-gcc $(CFLAGS) -nostartfiles ../src/crt0.o $(OBJS) -o $@ -Trx62n.ld

%.o : %.c
	rx-elf-gcc $(CFLAGS) -c $< -o $@

%.o : %.S
	rx-elf-gcc $(CFLAGS) -c $< -o $@

main.zip :
	zip -9 main.zip crt0.S *.c *.h rx62n.ld Makefile include/*

#-----------------------------------------------------------------------------

clean :
	-rm -f *.srec *.elf $(OBJS) *.d *.a *~

DEPS=$(wildcard *.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
