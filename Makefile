# Automatic makefile for GNUARM (C/C++)

#tutaj wpisz nazwe pliku hex
TARGET	   = isix

#Format wyjsciowy (moze byc srec,ihex,binary)
FORMAT	= ihex

#Optymalizacja [0,1,2,3,s]
# 0 - brak optymalizacji, s -optymalizacja rozmiaru
OPT 	= s

#Debug format 
DEBUG 	= stabs

#Typ procesora
MCU	= arm7tdmi

#Skrypt linkera
SCRIPTLINK = lpc2142-rom

#Opcje kompilatora C
CFLAGS += -Wall
CFLAGS += -std=gnu99 -fomit-frame-pointer

CFLAGS += -Iinclude


#Opcje kompilatora C++
CXXFLAGS += -Wall -fomit-frame-pointer



#Port szeregowy programatora ISP
ISPPORT = /dev/ttyS0

#Predkosc portu szeregowego programatora
ISPBAUD = 19200

#Czestotliwosc rezonatora kwarcowego mikrokontrolera (kHz)
ISPXTAL = 12000


#Definicje programow
CC      = arm-elf-gcc
CXX	= arm-elf-c++
AR      = arm-elf-ar
CP      = arm-elf-objcopy
OBJDUMP = arm-elf-objdump 
ISPPROG  = lpc21isp


#Pozostale ustawienia kompilatora

ASFLAGS += -Wa,-mapcs-32 -mcpu=$(MCU) -g$(DEBUG)
LDFLAGS +=  -nostartfiles -T$(SCRIPTLINK).ld -Wl,-Map=$(TARGET).map,--cref
CFLAGS  += -O$(OPT) -mcpu=$(MCU) -g$(DEBUG) 
CXXFLAGS += -O$(OPT) -mcpu=$(MCU) -g$(DEBUG) 
CPFLAGS =  -O $(FORMAT) -S
ASLST = -Wa,-adhlns=$(<:.S=.lst)
CLST = -Wa,-adhlns=$(<:.c=.lst)
CPPLST = -Wa,-adhlns=$(<:.cpp=.lst)

all:	build

install: build program

clean:
	rm -f $(TARGET).hex
	rm -f $(TARGET).elf
	rm -f $(TARGET).map
	rm -f $(TARGET).lss
	rm -f $(OBJ)
	rm -f $(ASRC:.S=.lst) $(SRC:.c=.lst) $(CPPSRC:.cpp=.lst)
	rm -f $(SRC:.c=.dep) $(CPPSRC:.cpp=.dep) $(ASRC:.S=.dep)

program:
	$(ISPPROG) -control $(TARGET).hex $(ISPPORT) $(ISPBAUD) $(ISPXTAL)

     
build:	$(TARGET).elf $(TARGET).hex $(TARGET).lss



#pliki zrodlowe C
SRC = $(wildcard *.c)
#pliki zrodlowe C++
CPPSRC = $(wildcard *.cpp)
#pliki assemblerowe
ASRC = $(wildcard *.S) 


#wszystkie zaleznosci
$(TARGET).elf: $(OBJ) $(SCRIPTLINK).ld
-include $(SRC:.c=.dep)
-include $(CPPSRC:.cpp=.dep)
-include $(ASRC:.S=.dep)

 
#Objects files
OBJ = $(SRC:.c=.o) $(CPPSRC:.cpp=.o) $(ASRC:.S=.o)
# Define all listing files.
LST = $(SRC:.c=.lst) $(CPPSRC:.cpp=.lst) $(ASRC:.S=.lst)

include Makefile.inc
