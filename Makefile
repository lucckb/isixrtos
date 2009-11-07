########################################################################
######              Top makefile project for the ISIX OS          ######
######         Copyright (c) 2009 Lucjan Bryndza                  ######
########################################################################
# Isix os Makefile 
#Katalog glowny
TOP_DIR= $(shell pwd)

#tutaj wpisz nazwe pliku hex
TARGET	   = isix


#Optymalizacja [0,1,2,3,s]
# 0 - brak optymalizacji, s -optymalizacja rozmiaru
OPT 	= 0

#Debug format 
DEBUG 	= y


#Typ procesora
MCU	= cortex-m3


#Skrypt linkera
SCRIPTLINK = arch/arm7lpc2000/boot/lpc2148-rom

#Opcje kompilatora C
CFLAGS += -Wall
CFLAGS += -std=gnu99

CFLAGS += -I$(TOP_DIR)/include


CROSS_COMPILE ?= arm-elf-



#Definicje programow
CC      = $(CROSS_COMPILE)gcc
AR      = $(CROSS_COMPILE)ar
CP      = $(CROSS_COMPILE)objcopy
LD      = $(CROSS_COMPILE)ld
OBJDUMP = $(CROSS_COMPILE)objdump 



#Pozostale ustawienia kompilatora

ASFLAGS += -Wa,-mapcs-32 -mcpu=$(MCU) -O$(OPT) -mthumb
LDFLAGS +=  -nostartfiles -nostdlib -lgcc -T$(SCRIPTLINK).ld -Wl,-Map=$(TARGET).map,--cref -O$(OPT) -mthumb
CFLAGS  += -mcpu=$(MCU) -O$(OPT) -mthumb
CPFLAGS =  -O $(FORMAT) -S
ARFLAGS = rcs

ifeq ($(DEBUG),y)
CFLAGS += -g
LDFLAGS += -g
ASFLAGS += -gstabs
else
CFLAGS += -fomit-frame-pointer 
LDFLAGS += -fomit-frame-pointer
ASFLAGS += -fomit-frame-pointer
endif


#eksportujemy potrzebne dane
export ASFLAGS LDFLAGS CFLAGS ARFLAGS TOP_DIR CC AR LD

all:	build target


install: build target 


clean:
	find $(TOP_DIR) -name '*.o' | xargs rm -f
	find $(TOP_DIR) -name '*.dep' | xargs rm -f
	rm -f *.lss *.map *.a


target:	lib$(TARGET).a

build:	
	$(MAKE) -C kernel
	$(MAKE) -C arch/cm3stm32


LINKFILES =  kernel/kernel.o arch/cm3stm32/kernel_arch.o 


lib$(TARGET).a: $(LINKFILES)
	@echo "Creating static lib.."
	$(AR) $(ARFLAGS) $@ $(LINKFILES)

include Makefile.inc

