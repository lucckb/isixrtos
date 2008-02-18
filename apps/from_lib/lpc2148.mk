# Automatic makefile for GNUARM (C/C++)

#Typ procesora
MCU	= arm7tdmi-s

#Skrypt linkera
SCRIPTLINK = lpc2148-rom


#Definicje programow
CC      = arm-elf-gcc
CXX	= arm-elf-c++
AR      = arm-elf-ar
CP      = arm-elf-objcopy
OBJDUMP = arm-elf-objdump 
JTAGPROG  = openocd

OCDSCRIPT=/tmp/openocd.script

ISIX_INC = ../../include
ISIX_LIB = ../../

LSCRIPT = ../../arch/arm7lpc2000/boot/$(SCRIPTLINK).ld


CFLAGS += -I$(ISIX_INC)
CXXFLAGS += -I$(ISIX_INC)

#Pozostale ustawienia kompilatora

ASFLAGS += -Wa,-mapcs-32 -mcpu=$(MCU)
LDFLAGS +=  -nostartfiles -T$(LSCRIPT) -Wl,-Map=$(TARGET).map,--cref -L$(ISIX_LIB) -lisix
CFLAGS  += -O$(OPT) -mcpu=$(MCU)
CXXFLAGS += -O$(OPT) -mcpu=$(MCU)
CPFLAGS =  -S


ifeq ($(LISTING),y)
ASLST = -Wa,-adhlns=$(<:.S=.lst)
CLST = -Wa,-adhlns=$(<:.c=.lst)
CPPLST = -Wa,-adhlns=$(<:.cpp=.lst)
LSSTARGET = $(TARGET).lss
endif

ifeq ($(DEBUG),y)
CFLAGS += -g
CXXFLAGS += -g
LDFLAGS += -g
ASFLAGS += -gstabs
else
CFLAGS += -fomit-frame-pointer
CXXFLAGS += -fomit-frame-pointer
LDFLAGS += -fomit-frame-pointer
ASFLAGS += -fomit-frame-pointer
endif

ifneq ($(FORMAT),elf)
	CONVTARGET = $(TARGET).$(FORMAT)
endif


all:	build

install: build program

clean:
	rm -f $(TARGET).hex
	rm -f $(TARGET).elf
	rm -f $(TARGET).map
	rm -f $(TARGET).lss
	rm -f $(TARGET).bin
	rm -f $(OBJ)
	rm -f $(ASRC:.S=.lst) $(SRC:.c=.lst) $(CPPSRC:.cpp=.lst)
	rm -f $(SRC:.c=.dep) $(CPPSRC:.cpp=.dep) $(ASRC:.S=.dep)

program:
	lpc21isp  $(TARGET).hex /dev/ttyUSB0 19200 12000 || true
     
build:	$(TARGET).elf $(CONVTARGET) $(LSSTARGET)


#pliki zrodlowe C
SRC = $(wildcard *.c)

#pliki zrodlowe C++
CPPSRC = $(wildcard *.cpp)

#pliki assemblerowe
ASRC = $(wildcard *.S)


#wszystkie zaleznosci
$(TARGET).elf: $(OBJ) $(LSCRIPT)
-include $(SRC:.c=.dep)
-include $(CPPSRC:.cpp=.dep)
-include $(ASRC:.S=.dep)

 
#Objects files
OBJ = $(SRC:.c=.o) $(CPPSRC:.cpp=.o) $(ASRC:.S=.o)
# Define all listing files.
LST = $(SRC:.c=.lst) $(CPPSRC:.cpp=.lst) $(ASRC:.S=.lst)


%.dep: %.c
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.dep: %.cpp 
	$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.dep: %.S
	$(CC) -M $(ASFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.lss: %.elf
	@echo "Create extended listing..."
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	@echo "Converting to hex..."
	$(CP) -O ihex $(CPFLAGS) $< $@ 

%.bin: %.elf
	@echo "Converting to bin..."
	$(CP) -O binary $(CPFLAGS) $< $@ 

%.elf: $(OBJ)
	@echo "Linking..."
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@ $(LDFLAGS)

%.o : %.S
	@echo "Assembling..."
	$(CC) -c $(ASFLAGS) $(ASLST) $< -o $@ 


%.o : %.c	
	@echo "Compiling C..."
	$(CC) -c $(CFLAGS) $(CLST) $< -o $@

%.o : %.cpp
	@echo "Compiling C++..."
	$(CXX) -c $(CXXFLAGS) $(CPPLST) $< -o $@

