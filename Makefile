# Isix os Makefile 
#Katalog glowny
TOP_DIR= $(shell pwd)

#tutaj wpisz nazwe pliku hex
TARGET	   = isix

#Format wyjsciowy (moze byc srec,ihex,binary)
FORMAT	= ihex

#Optymalizacja [0,1,2,3,s]
# 0 - brak optymalizacji, s -optymalizacja rozmiaru
OPT 	= s

#Debug format 
DEBUG 	= stabs

#Czy programowanie isp czy nie
ISP = y

#Typ procesora
MCU	= arm7tdmi

#Malloc heap end (END MEM - TOP STACK)
HEAP_END = 0x40003CF4



#Skrypt linkera
SCRIPTLINK = arch/arm7lpc2000/boot/lpc2142-rom

#Opcje kompilatora C
CFLAGS += -Wall
CFLAGS += -std=gnu99 -fomit-frame-pointer

CFLAGS += -I$(TOP_DIR)/include


ARCH ?= arm-elf


#Port szeregowy programatora ISP
ISPPORT = /dev/ttyS0

#Predkosc portu szeregowego programatora
ISPBAUD = 19200

#Czestotliwosc rezonatora kwarcowego mikrokontrolera (kHz)
ISPXTAL = 12000

#tymczasowy katalog dla skryptow
TMPSCRIPT = /tmp/pgm.script

#Definicje programow
CC      = $(ARCH)-gcc
AR      = $(ARCH)-ar
CP      = $(ARCH)-objcopy
LD      = $(ARCH)-ld
OBJDUMP = $(ARCH)-objdump 


ISPPROG  = lpc21isp
JTAGPROG = openocd


#Pozostale ustawienia kompilatora

ASFLAGS += -Wa,-mapcs-32 -mcpu=$(MCU) -g$(DEBUG)
LDFLAGS +=  -nostartfiles -nostdlib -lgcc -T$(SCRIPTLINK).ld -Wl,-Map=$(TARGET).map,--cref,--defsym=__heap_end=$(HEAP_END)
CFLAGS  += -O$(OPT) -mcpu=$(MCU) -g$(DEBUG) 
CPFLAGS =  -O $(FORMAT) -S
ARFLAGS = rcs

#eksportujemy potrzebne dane
export ASFLAGS LDFLAGS CFLAGS ARFLAGS TOP_DIR CC AR LD

all:	build target

install: build target program

clean:
	find $(TOP_DIR) -name '*.o' | xargs rm -f
	find $(TOP_DIR) -name '*.dep' | xargs rm -f
	rm -rf *.hex *.elf *.lss *.map

program:
    ifeq ( $(ISP) ,y )
	$(ISPPROG) -control $(TARGET).hex $(ISPPORT) $(ISPBAUD) $(ISPXTAL) || true
	@ echo " "
    else
	echo "arm7_9 dcc_downloads enable" > $(TMPSCRIPT)
	echo "wait_halt" >> $(TMPSCRIPT)
	echo "sleep 10" >> $(TMPSCRIPT)
	echo "poll" >> $(TMPSCRIPT)
	echo "flash probe 0" >> $(TMPSCRIPT)
	echo "flash erase 0 0 6" >> $(TMPSCRIPT)
	echo "flash write_image "$(TOP_DIR)"/"$(TARGET)".hex ihex" >> $(TMPSCRIPT)
	echo "reset run" >> $(TMPSCRIPT)
	echo "sleep 10"  >> $(TMPSCRIPT)
	echo "shutdown" >> $(TMPSCRIPT)
	$(JTAGPROG) -f lpc2148.cfg
    endif

target: $(TARGET).elf $(TARGET).hex $(TARGET).lss

build:	
	$(MAKE) -C kernel
	$(MAKE) -C arch/arm7lpc2000
	$(MAKE) -C apps/isixtest


LINKFILES =  kernel/kernel.o arch/arm7lpc2000/kernel_arch.o apps/isixtest/isixtest.o

#wszystkie zaleznosci
$(TARGET).elf: $(LINKFILES) $(SCRIPTLINK).ld
	@echo "Linking..."
	$(CC) $(CFLAGS) $(LINKFILES) -o $@ $(LDFLAGS)


include Makefile.inc

%.lss: %.elf
	@echo "Create extended listing..."
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	@echo "Converting to hex..."
	$(CP) $(CPFLAGS) $< $@ 

