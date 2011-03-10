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


CXXFLAGS += -fno-rtti -fcheck-new -fno-exceptions

#Pozostale ustawienia kompilatora

ASFLAGS += -Wa,-mapcs-32 -mcpu=$(MCU) -mthumb
LDFLAGS +=  -nostartfiles -T$(LSCRIPT) -Wl,-Map=$(TARGET).map,--cref -mthumb
CFLAGS  += -O$(OPT) -mcpu=$(MCU) -mthumb
CXXFLAGS += -O$(OPT) -mcpu=$(MCU) -mthumb
CPFLAGS =  -S
ARFLAGS = rcs

ifeq ($(LISTING),y)
ASLST = -Wa,-adhlns=$(<:%.S=%.lst)
CLST = -Wa,-adhlns=$(<:%.c=%.lst)
CPPLST = -Wa,-adhlns=$(<:%.cpp=%.lst)
LSSTARGET = $(TARGET).lss
endif

ifeq ($(DEBUG),y)
CFLAGS += -g -DPDEBUG
CXXFLAGS += -g -DPDEBUG
LDFLAGS += -g -DPDEBUG
ASFLAGS += -gstabs -DPDEBUG
else
CFLAGS += -fomit-frame-pointer -flto
CXXFLAGS += -fomit-frame-pointer -flto
LDFLAGS += -fomit-frame-pointer -flto
ASFLAGS += -fomit-frame-pointer -flto
#Remove unused functions 
CFLAGS += -ffunction-sections -fdata-sections
CXXFLAGS += -ffunction-sections -fdata-sections
LDFLAGS+= -Wl,--gc-sections
endif

all:	build

install: build program

clean:
	rm -f $(TARGET).$(FORMAT)
	rm -f $(TARGET).elf
	rm -f $(TARGET).map
	rm -f $(TARGET).lss
	rm -f lib$(TARGET).a
	rm -f $(OBJ) $(LST) $(DEPFILES)


program:
	openocd -f $(SCRIPTS_DIR)/stm32.cfg -c init -c 'script $(SCRIPTS_DIR)/flash-begin-$(MCU_VARIANT).script' \
	-c "flash write_image $(TARGET).elf" -c 'script $(SCRIPTS_DIR)/flash-end-$(MCU_VARIANT).script' \
	-c shutdown || true

devrst:
	openocd -f $(SCRIPTS_DIR)/stm32.cfg -c init -c reset run -c shutdown

ifeq ($(LIBRARY),y)
build:	target  
else
build:	target $(LSSTARGET) size-calc
endif



ifeq ($(LIBRARY),y)
target:	lib$(TARGET).a
else
target: $(TARGET).elf $(TARGET).$(FORMAT) $(TARGET).lss
endif

#Calculate size
size-calc: $(TARGET).elf
	$(SIZE) $<
	

#wszystkie zaleznosci
$(TARGET).elf: $(OBJ) $(LSCRIPT) 
#Tworzenie biblioteki
lib$(TARGET).a: $(OBJ)

-include $(SRC:%.c=%.dep)
-include $(CPPSRC:%.cpp=%.dep)
-include $(ASRC:%.S=%.dep)
 
 
#Objects files
OBJ = $(SRC:%.c=%.o) $(CPPSRC:%.cpp=%.o) $(ASRC:%.S=%.o)
# Define all listing files.
LST = $(SRC:%.c=%.lst) $(CPPSRC:%.cpp=%.lst) $(ASRC:%.S=%.lst)
#Depend files
DEPFILES = $(SRC:%.c=%.dep) $(CPPSRC:%.cpp=%.dep) $(ASRC:%.S=%.dep)

#Objects files
.PRECIOUS : $(OBJ)
ifeq ($(LIBRARY),y)
.SECONDARY: lib$(TARGET).a
else
.SECONDARY: $(TARGET).elf
endif


%.dep: %.c
	$(CC) -MM -MF $@ -MP -MT $(subst .dep,.o,$@) $(CFLAGS) $< 

%.dep: %.cpp 
	$(CXX) -MM -MF $@ -MP -MT $(subst .dep,.o,$@) $(CXXFLAGS) $< 


%.dep: %.S
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

$(TARGET).elf: $(OBJ) $(CRT0_OBJECT) $(ADDITIONAL_DEPS)
	@echo "Linking..."
	$(CXX) $(CXXFLAGS) $(OBJ) $(CRT0_OBJECT) -o $@ $(LDFLAGS)

%.o : %.S
	@echo "Assembling..."
	$(CC) -c $(ASFLAGS) $(ASLST) $< -o $@ 


%.o : %.c	
	@echo "Compiling C..."
	$(CC) -c $(CFLAGS) $(CLST) $< -o $@

%.o : %.cpp
	@echo "Compiling C++..."
	$(CXX) -c $(CXXFLAGS) $(CPPLST) $< -o $@

lib$(TARGET).a : $(OBJ)
	@echo "Creating library ..."
	$(AR) $(ARFLAGS) $@ $(OBJ)

