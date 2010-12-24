
CFLAGS = -Wall -O2 -MMD -mint-register=4 -mlittle-endian-data -Iinclude

PATH := $(PATH):$(HOME)/m32c/install/bin

.PRECIOUS: %.o %.elf

BSPS = \
	clock.o \
	$(END_OF_LIST)

ELF = blinky2
OBJS = lcd.o font_x5x7.o interrupt_table.o

all : $(ELF).elf

flash : $(ELF).elf
	sudo rxusb -v $(ELF).elf

%.elf : crt0.o %.o $(OBJS) bsp.a rx62n.ld Makefile
	rx-elf-gcc $(CFLAGS) -nostartfiles crt0.o $*.o $(OBJS) bsp.a -o $@ -Trx62n.ld

os-0.elf : os-0.S Makefile rx62n.ld
	rx-elf-as -mlittle-endian-data os-0.S -o os-0.o
	rx-elf-ld -EL os-0.o -o os-0.elf -Trx62n.ld

bsp.a : $(BSPS)
	rm -f bsp.a
	ar rvs bsp.a $(BSPS)

%.o : %.c
	rx-elf-gcc $(CFLAGS) -c $< -o $@

%.o : %.S
	rx-elf-gcc $(CFLAGS) -c $< -o $@

blinky2.zip :
	zip -9 blinky2.zip crt0.S *.c *.h rx62n.ld Makefile include/*

#-----------------------------------------------------------------------------

clean :
	-rm -f *.srec *.elf *.o *.d *.a *~

DEPS=$(wildcard *.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
