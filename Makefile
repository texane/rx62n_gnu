
CFLAGS = -Wall -O2 -MMD -Wno-strict-aliasing -mint-register=4 -mlittle-endian-data -Iinclude

PATH := $(PATH):/opt/gnurx/bin/:$(HOME)/install/bin

.PRECIOUS: %.o %.elf

ELF = main
OBJS =				\
	hwsetup.o		\
	sched.o			\
	led_task.o		\
	swatch_task.o		\
	radar_task.o		\
	aversive.o can.o	\
	lcd.o font_x5x7.o	\
	interrupt_table.o

all : $(ELF).elf

flash : $(ELF).elf
	rxusb -v $(ELF).elf

%.elf : crt0.o %.o $(OBJS) rx62n.ld Makefile
	rx-elf-gcc $(CFLAGS) -nostartfiles crt0.o $*.o $(OBJS) -o $@ -Trx62n.ld

os-0.elf : os-0.S Makefile rx62n.ld
	rx-elf-as -mlittle-endian-data os-0.S -o os-0.o
	rx-elf-ld -EL os-0.o -o os-0.elf -Trx62n.ld

%.o : %.c
	rx-elf-gcc $(CFLAGS) -c $< -o $@

%.o : %.S
	rx-elf-gcc $(CFLAGS) -c $< -o $@

main.zip :
	zip -9 main.zip crt0.S *.c *.h rx62n.ld Makefile include/*

#-----------------------------------------------------------------------------

clean :
	-rm -f *.srec *.elf *.o *.d *.a *~

DEPS=$(wildcard *.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
