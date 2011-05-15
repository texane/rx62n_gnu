
CFLAGS = -Wall -O2 -MMD -Wno-strict-aliasing -mint-register=4 -mlittle-endian-data -I../src

PATH := $(PATH):/opt/gnurx/bin/:$(HOME)/install/bin

.PRECIOUS: %.o %.elf

ELF =	main
OBJS =				\
	../src/main.o		\
	../src/hwsetup.o	\
	../src/sched.o		\
	../src/switches.o	\
	../src/blinker.o	\
	../src/swatch.o		\
	../src/radar.o		\
	../src/aversive.o	\
	../src/can.o		\
	../src/lcd.o		\
	../src/font_x5x7.o	\
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
