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
CC_V     	:= $(CROSS_COMPILE)gcc
CXX_V	  	:= $(CROSS_COMPILE)g++
AR_V      	:= $(CROSS_COMPILE)ar
CP_V      	:= $(CROSS_COMPILE)objcopy
OBJDUMP_V 	:= $(CROSS_COMPILE)objdump 
SIZE_V 	  	:= $(CROSS_COMPILE)size
JTAGPROG_V  := openocd
RM_V		:= rm
LD_V		:= $(CROSS_COMPILE)g++
 
#Verbose stuff BEGIN
V?=0
CC_0 = @echo -e "   CC\t" $(@); $(CC_V)
CC_1 = $(CC_V)
CC = $(CC_$(V))

CXX_0 = @echo -e "   CXX\t" $(@); $(CXX_V)
CXX_1 = $(CXX_V)
CXX = $(CXX_$(V))

LD_0 = @echo -e "   LD\t" $(@); $(LD_V)
LD_1 = $(LD_V)
LD = $(LD_$(V))

AR_0 = @echo -e "   AR\t" $(@); $(AR_V)
AR_1 = $(AR_V)
AR = $(AR_$(V))

CP_0 = @echo -e "   CP\t" $(@); $(CP_V)
CP_1 = $(CP_V)
CP = $(CP_$(V))


OBJDUMP_0 = @echo -e "   OBJ\t" $(@); $(OBJDUMP_V)
OBJDUMP_1 = $(OBJDUMP_V)
OBJDUMP = $(OBJDUMP_$(V))

SIZE_0 := @$(SIZE_V)
SIZE_1 := $(SIZE_V)
SIZE =   $(SIZE_$(V))

JTAGPROG_0 = @echo -e "   PGM\t" $(@); $(JTAGPROG_V)
JTAGPROG_1 = $(JTAGPROG_V)
JTAGPROG = $(JTAGPROG_$(V))

RM_0 := @$(RM_V)
RM_1 := $(RM_V)
RM =   $(RM_$(V))

DEP_CC_0 = @$(CC_V)
DEP_CC_1 = $(CC_V)
DEP_CC = $(DEP_CC_$(V))

DEP_CXX_0 = @$(CXX_V)
DEP_CXX_1 = $(CXX_V)
DEP_CXX = $(DEP_CXX_$(V))

#Verbose stuff END


SCRIPTS_DIR := $(LIBSTM32_DIR)/scripts
LSCRIPT := $(SCRIPTS_DIR)/$(SCRIPTLINK).ld


#Pozostale ustawienia kompilatora
COMMON_FLAGS += -O$(OPT) -mcpu=$(MCU) -mthumb -Wno-variadic-macros
ifeq ($(MCU_MAJOR_TYPE),f4)
COMMON_FLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16 -ffast-math -fsingle-precision-constant
COMMON_FLAGS += -DSTM32MCU_MAJOR_TYPE_F4
else
ifeq ($(MCU_MAJOR_TYPE),f2)
COMMON_FLAGS += -DSTM32MCU_MAJOR_TYPE_F2
else
COMMON_FLAGS += -DSTM32MCU_MAJOR_TYPE_F1
endif
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
endif
ifeq ($(MCU_MAJOR_TYPE),f2)
OCDSCRIPT_FILE=stm32f2x.cfg
endif
ifeq ($(MCU_MAJOR_TYPE),f1)
OCDSCRIPT_FILE=stm32.cfg
endif
endif
OCDSCRIPT_FILE ?= stm32.cfg

ifdef MCU_SYSTEM_STACK_SIZE
LDFLAGS += -Wl,--defsym=_sys_stack_size=$(MCU_SYSTEM_STACK_SIZE)
endif

#TODO: Enable it later 
#CFLAGS += -Wc++-compat

all:	build

install: build program

.PHONY : clean
clean:
	@echo Cleaning project $(TARGET)
	$(RM) -f $(TARGET).$(FORMAT)
	$(RM) -f $(TARGET).elf
	$(RM) -f $(TARGET).map
	$(RM) -f $(TARGET).lss
	$(RM) -f lib$(TARGET).a
	$(RM) -f $(OBJ) $(LST) $(DEPFILES) $(LIBS) $(LIBS_OBJS)


program: $(TARGET).elf
	$(JTAGPROG) -f $(SCRIPTS_DIR)/$(OCDSCRIPT_FILE) -c init -c 'script $(SCRIPTS_DIR)/flash-begin-$(MCU_VARIANT).script' \
	-c "flash write_image erase unlock $(TARGET).elf" -c 'script $(SCRIPTS_DIR)/flash-end-$(MCU_VARIANT).script' \
	-c shutdown || true

.PHONY : devrst
devrst:
	$(JTAGPROG) -f $(SCRIPTS_DIR)/$(OCDSCRIPT_FILE) -c init -c reset run -c shutdown


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

.ONESHELL:
%.dep: %.c
	$(DEP_CC) -MM -MF $@ -MP -MT $(subst .dep,.o,$@) $(CFLAGS) $< 

.ONESHELL:
%.dep: %.cpp 
	$(DEP_CXX) -MM -MF $@ -MP -MT $(subst .dep,.o,$@) $(CXXFLAGS) $< 

.ONESHELL:
%.dep: %.S
	$(DEP_CC) -MM -MF $@ -MP -MT $(subst .dep,.o,$@) $(ASFLAGS) $< 

.ONESHELL:
%.lss: %.elf
	$(OBJDUMP) -h -S $< > $@

.ONESHELL:
%.hex: %.elf
	$(CP) -O ihex $(CPFLAGS) $< $@ 

.ONESHELL:
%.bin: %.elf
	$(CP) -O binary $(CPFLAGS) $< $@ 

.ONESHELL:
$(TARGET).elf: $(OBJ) $(CRT0_OBJECTS) $(ADDITIONAL_DEPS) $(LIBS)
	$(LD) $(CXXFLAGS) $(OBJ) $(CRT0_OBJECTS) $(LIBS) -o $@ $(LDFLAGS) $(LINK_LIBS)

.ONESHELL:
%.o : %.S
	$(CC) -c $(ASFLAGS) $(ASLST) $< -o $@ 

.ONESHELL:
%.o : %.c	
	$(CC) -c $(CFLAGS) $(CLST) $< -o $@

.ONESHELL:
%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPLST) $< -o $@

.ONESHELL:
lib$(TARGET).a : $(OBJ)
	$(AR) $(ARFLAGS) $@ $(OBJ)

