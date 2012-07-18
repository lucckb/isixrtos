# Automatic makefile for GNUARM (C/C++)


#Typ procesora
ifeq ($(MCU_MAJOR_TYPE),f4)
MCU	= cortex-m4
else
MCU	= cortex-m3
endif

#Old MCU variant now is defined as minor major CPU code
MCU_VARIANT ?= $(MCU_MAJOR_TYPE)$(MCU_MINOR_TYPE)

#Skrypt linkera
SCRIPTLINK = stm32-$(MCU_VARIANT)


#CROSS COMPILE
CROSS_COMPILE ?= arm-none-eabi-
#Definicje programow
CC      = $(CROSS_COMPILE)gcc
CXX	= $(CROSS_COMPILE)g++
AR      = $(CROSS_COMPILE)ar
CP      = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump 
SIZE = $(CROSS_COMPILE)size
JTAGPROG  = openocd

OCDSCRIPT=/tmp/pgm.script
OCD=openocd

SCRIPTS_DIR = $(LIBSTM32_DIR)/scripts

LSCRIPT = $(SCRIPTS_DIR)/$(SCRIPTLINK).ld


#Pozostale ustawienia kompilatora
COMMON_FLAGS += -O$(OPT) -mcpu=$(MCU) -mthumb -Wno-variadic-macros
ifeq ($(MCU_MAJOR_TYPE),f4)
COMMON_FLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16 -ffast-math -fsingle-precision-constant
COMMON_FLAGS += -DSTM32MCU_MAJOR_TYPE_F4
else
COMMON_FLAGS += -DSTM32MCU_MAJOR_TYPE_F1
endif
ASFLAGS += -Wa,-mapcs-32 -mcpu=$(MCU) -mthumb
LDFLAGS +=  -L$(SCRIPTS_DIR) -nostdlib -nostartfiles -T$(LSCRIPT) -Wl,-Map=$(TARGET).map,--cref -mthumb
CPFLAGS =  -S
ARFLAGS = rcs

LINK_LIBS = -Wl,--start-group -lstdc++ -lc -lm -lg -lgcc -Wl,--end-group
#If cpp exceptions is suported
ifeq ($(CPP_EXCEPTIONS),y)
COMMON_FLAGS += -DCONFIG_ENABLE_EXCEPTIONS
else
CXXFLAGS += -fno-exceptions -fno-rtti
endif

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
CFLAGS += -fomit-frame-pointer 
CXXFLAGS += -fomit-frame-pointer
LDFLAGS += -fomit-frame-pointer
ASFLAGS += -fomit-frame-pointer
#Remove unused functions 
CFLAGS += -ffunction-sections -fdata-sections
CXXFLAGS += -ffunction-sections -fdata-sections
LDFLAGS+= -Wl,--gc-sections
endif
CXXFLAGS+= $(COMMON_FLAGS)
CFLAGS+= $(COMMON_FLAGS)

ifeq ($(SMALL_WORK_AREA),y)
OCDSCRIPT_FILE=stm32small.cfg
else
ifeq ($(MCU_MAJOR_TYPE),f4)
OCDSCRIPT_FILE=stm32f4x.cfg
else
OCDSCRIPT_FILE=stm32.cfg
endif
endif


all:	build

install: build program

clean:
	rm -f $(TARGET).$(FORMAT)
	rm -f $(TARGET).elf
	rm -f $(TARGET).map
	rm -f $(TARGET).lss
	rm -f lib$(TARGET).a
	rm -f $(OBJ) $(LST) $(DEPFILES) $(LIBS) $(LIBS_OBJS)


program:
	openocd -f $(SCRIPTS_DIR)/$(OCDSCRIPT_FILE) -c init -c 'script $(SCRIPTS_DIR)/flash-begin-$(MCU_VARIANT).script' \
	-c "flash write_image erase unlock $(TARGET).elf" -c 'script $(SCRIPTS_DIR)/flash-end-$(MCU_VARIANT).script' \
	-c shutdown || true

devrst:
	openocd -f $(SCRIPTS_DIR)/$(OCDSCRIPT_FILE) -c init -c reset run -c shutdown

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

#Depend files
DEPFILES += $(SRC:%.c=%.dep) $(CPPSRC:%.cpp=%.dep) $(ASRC:%.S=%.dep)

-include $(DEPFILES)
 
 
#Objects files
OBJ = $(SRC:%.c=%.o) $(CPPSRC:%.cpp=%.o) $(ASRC:%.S=%.o)
# Define all listing files.
LST = $(SRC:%.c=%.lst) $(CPPSRC:%.cpp=%.lst) $(ASRC:%.S=%.lst)

#Objects files
$.PRECIOUS : $(OBJ)
ifeq ($(LIBRARY),y)
.SECONDARY: lib$(TARGET).a
else
.SECONDARY: $(TARGET).elf
endif

.DEFAULT_GOAL := all

%.dep: %.c
	$(CC) -MM -MF $@ -MP -MT $(subst .dep,.o,$@) $(CFLAGS) $< 

%.dep: %.cpp 
	$(CXX) -MM -MF $@ -MP -MT $(subst .dep,.o,$@) $(CXXFLAGS) $< 


%.dep: %.S
	$(CC) -MM -MF $@ -MP -MT $(subst .dep,.o,$@) $(ASFLAGS) $< 


%.lss: %.elf
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	$(CP) -O ihex $(CPFLAGS) $< $@ 

%.bin: %.elf
	$(CP) -O binary $(CPFLAGS) $< $@ 

$(TARGET).elf: $(OBJ) $(CRT0_OBJECTS) $(ADDITIONAL_DEPS) $(LIBS)
	$(CXX) $(CXXFLAGS) $(OBJ) $(CRT0_OBJECTS) $(LIBS) -o $@ $(LDFLAGS) $(LINK_LIBS)

%.o : %.S
	$(CC) -c $(ASFLAGS) $(ASLST) $< -o $@ 


%.o : %.c	
	$(CC) -c $(CFLAGS) $(CLST) $< -o $@

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPLST) $< -o $@


lib$(TARGET).a : $(OBJ)
	$(AR) $(ARFLAGS) $@ $(OBJ)

