# Automatic makefile for GNUARM (C/C++)

#Typ procesora
MCU	= cortex-m3

#Skrypt linkera
SCRIPTLINK = stm32-$(MCU_VARIANT)


#CROSS COMPILE
CROSS_COMPILE ?= arm-none-eabi-
#Definicje programow
CC      = $(CROSS_COMPILE)gcc
CXX	= $(CROSS_COMPILE)c++
AR      = $(CROSS_COMPILE)ar
CP      = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump 
SIZE = $(CROSS_COMPILE)size
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
CFLAGS += -fomit-frame-pointer
CXXFLAGS += -fomit-frame-pointer
LDFLAGS += -fomit-frame-pointer
ASFLAGS += -fomit-frame-pointer
#Remove unused functions 
CFLAGS += -ffunction-sections -fdata-sections
CXXFLAGS += -ffunction-sections -fdata-sections
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
	openocd -f $(SCRIPTS_DIR)/stm32.cfg -c init -c 'script $(SCRIPTS_DIR)/flash-begin-$(MCU_VARIANT).script' \
	-c "flash write_image $(TARGET).elf" -c 'script $(SCRIPTS_DIR)/flash-end-$(MCU_VARIANT).script' \
	-c shutdown || true

devrst:
	openocd -f $(SCRIPTS_DIR)/stm32.cfg -c init -c reset run -c shutdown

ifeq ($(LIBRARY),y)
build:	target  
else
build:	target $(CONVTARGET) $(LSSTARGET) size-calc
endif


#Create object directory
$(OBJDIR):
		mkdir -p $@

ifeq ($(LIBRARY),y)
target:	lib$(TARGET).a
else
target: $(TARGET).elf $(TARGET).hex $(TARGET).lss
endif

#Calculate size
size-calc: $(TARGET).elf
	$(SIZE) $<
	

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
	$(CC) -MM -MF $@ -MP -MT $(subst .dep,.o,$@) $(CFLAGS) $< 


$(OBJDIR)/%.dep: %.cpp 
	$(CXX) -MM -MF $@ -MP -MT $(subst .dep,.o,$@) $(CXXFLAGS) $< 


$(OBJDIR)/%.dep: %.S
	$(CC) -MM -MF $@ -MP -MT $(subst .dep,.o,$@) $(ASFLAGS) $< 


%.lss: %.elf
	@echo "Create extended listing..."
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	@echo "Converting to hex..."
	$(CP) -O ihex $(CPFLAGS) $< $@ 

%.bin: %.elf
	@echo "Converting to bin..."
	$(CP) -O binary $(CPFLAGS) $< $@ 

%.elf: $(OBJ) $(CRT0_OBJECT) $(ADDITIONAL_DEPS)
	@echo "Linking..."
	$(CXX) $(CXXFLAGS) $(OBJ) $(CRT0_OBJECT) -o $@ $(LDFLAGS)

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

