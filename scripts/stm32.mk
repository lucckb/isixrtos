# Automatic makefile for GNUARM (C/C++)

#Typ procesora
MCU	= cortex-m3

#Skrypt linkera
SCRIPTLINK = stm32


#CROSS COMPILE
CROSS_COMPILE ?= arm-none-eabi-
#Definicje programow
CC      = $(CROSS_COMPILE)gcc
CXX	= $(CROSS_COMPILE)c++
AR      = $(CROSS_COMPILE)ar
CP      = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump 
JTAGPROG  = openocd

OCDSCRIPT=/tmp/pgm.script
OCD=openocd

STRLIB_INC = ../lib-stm32/inc/

SCRIPTS_DIR = ../lib-stm32/scripts

LSCRIPT = $(SCRIPTS_DIR)/$(SCRIPTLINK).ld


CFLAGS += -I$(STRLIB_INC)
CXXFLAGS += -I$(STRLIB_INC) -fno-rtti -fcheck-new -fno-exceptions

#dep
OBJDIR=obj

#Pozostale ustawienia kompilatora

ASFLAGS += -Wa,-mapcs-32 -mcpu=$(MCU) -mthumb
LDFLAGS +=  -nostartfiles -T$(LSCRIPT) -Wl,-Map=$(TARGET).map,--cref -mthumb
CFLAGS  += -O$(OPT) -mcpu=$(MCU) -mthumb
CXXFLAGS += -O$(OPT) -mcpu=$(MCU) -mthumb
CPFLAGS =  -S
ARFLAGS = rcs

ifeq ($(LISTING),y)
ASLST = -Wa,-adhlns=$(<:%.S=$(OBJDIR)/%.lst)
CLST = -Wa,-adhlns=$(<:%.c=$(OBJDIR)/%.lst)
CPPLST = -Wa,-adhlns=$(<:%.cpp=$(OBJDIR)/%.lst)
LSSTARGET = $(TARGET).lss
endif

ifeq ($(DEBUG),y)
CFLAGS += -g -DPDEBUG
CXXFLAGS += -g -DPDEBUG
LDFLAGS += -g -DPDEBUG
ASFLAGS += -gstabs -DPDEBUG
else
CFLAGS += -fomit-frame-pointer -ffunction-sections
CXXFLAGS += -fomit-frame-pointer -ffunction-sections -fvtable-gc
LDFLAGS += -fomit-frame-pointer -Wl,-gc-sections
ASFLAGS += -fomit-frame-pointer -ffunction-sections
#Remove unused functions 
CFLAGS += -ffunction-sections -fdata-sections
CXXFLAGS += -ffunction-sections -fdata-sections -fvtable-gc
LDFLAGS+= -Wl,--gc-sections
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
	rm -f lib$(TARGET).a
	rm -rf $(OBJDIR)


program:
	openocd -f $(SCRIPTS_DIR)/stm32.cfg -c init -c 'script $(SCRIPTS_DIR)/flash-begin.script' \
	-c "flash write_image $(TARGET).elf" -c 'script $(SCRIPTS_DIR)/flash-end.script' \
	-c shutdown || true

devrst:
	openocd -f $(SCRIPTS_DIR)/stm32.cfg -c init -c reset run -c shutdown

ifeq ($(LIBRARY),y)
build:	target  
else
build:	target $(CONVTARGET) $(LSSTARGET)
endif


#Create object directory
$(OBJDIR):
		mkdir -p $@

ifeq ($(LIBRARY),y)
target:	lib$(TARGET).a
else
target: $(TARGET).elf $(TARGET).hex $(TARGET).lss
endif


#wszystkie zaleznosci
$(TARGET).elf: $(OBJDIR) $(OBJ) $(LSCRIPT) 
#Tworzenie biblioteki
lib$(TARGET).a: $(OBJDIR) $(OBJ)

-include $(shell mkdir -p $(OBJDIR))
-include $(SRC:%.c=$(OBJDIR)/%.dep)
-include $(CPPSRC:%.cpp=$(OBJDIR)/%.dep)
-include $(ASRC:%.S=$(OBJDIR)/%.dep)
 
#Objects files
OBJ = $(SRC:%.c=$(OBJDIR)/%.o) $(CPPSRC:%.cpp=$(OBJDIR)/%.o) $(ASRC:%.S=$(OBJDIR)/%.o)
# Define all listing files.
LST = $(SRC:%.c=$(OBJDIR)/%.lst) $(CPPSRC:%.cpp=$(OBJDIR)/%.lst) $(ASRC:%.S=$(OBJDIR)/%.lst)
#Objects files

.PRECIOUS : $(OBJ)
ifeq ($(LIBRARY),y)
.SECONDARY: lib$(TARGET).a
else
.SECONDARY: $(TARGET).elf
endif

$(OBJDIR)/%.dep: %.c
	$(CC) -M $(CFLAGS) $< > $@; \
	sed -e 's/\(^$(*F)\.o\)/$(OBJDIR)\/\1/g' -i $@; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' -i $@


$(OBJDIR)/%.dep: %.cpp 
	$(CXX) -M $(CXXFLAGS) $< > $@; \
	sed -e 's/\(^$(*F)\.o\)/$(OBJDIR)\/\1/g' -i $@; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' -i $@


$(OBJDIR)/%.dep: %.S
	$(CC) -M $(ASFLAGS) $< > $@; \
	sed -e 's/\(^$(*F)\.o\)/$(OBJDIR)\/\1/g' -i $@; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' -i $@


%.lss: %.elf
	@echo "Create extended listing..."
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	@echo "Converting to hex..."
	$(CP) -O ihex $(CPFLAGS) $< $@ 

%.bin: %.elf
	@echo "Converting to bin..."
	$(CP) -O binary $(CPFLAGS) $< $@ 

%.elf: $(OBJ) $(ADDITIONAL_DEPS)
	@echo "Linking..."
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o : %.S
	@echo "Assembling..."
	$(CC) -c $(ASFLAGS) $(ASLST) $< -o $@ 


$(OBJDIR)/%.o : %.c	
	@echo "Compiling C..."
	$(CC) -c $(CFLAGS) $(CLST) $< -o $@

$(OBJDIR)/%.o : %.cpp
	@echo "Compiling C++..."
	$(CXX) -c $(CXXFLAGS) $(CPPLST) $< -o $@

ifeq ($(LIBRARY),y)
%.a : $(OBJ)
	@echo "Creating library ..."
	$(AR) $(ARFLAGS) $@ $(OBJ)
endif

