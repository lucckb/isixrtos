# Automatic makefile for GNUARM (C/C++)

#Old MCU variant now is defined as minor major CPU code
MCU_VARIANT = $(MCU_MAJOR_TYPE)$(MCU_MINOR_TYPE)

#Linker scripts config
ifeq ($(USE_SBL_BOOTLOADER),y)
SCRIPTLINK = stm32-$(MCU_VARIANT)-sbl
else
SCRIPTLINK = stm32-$(MCU_VARIANT)
endif

#Check for ccache
CCACHE_VER := $(shell ccache --version 2> /dev/null )

#CROSS COMPILE
CROSS_COMPILE ?= arm-none-eabi-

ifdef CCACHE_VER 
CROSS_COMPILE_0 = ccache $(CROSS_COMPILE)
else
CROSS_COMPILE_0 = $(CROSS_COMPILE)
endif


# Cross compile stuff
CC_V     	:= $(CROSS_COMPILE_0)gcc
CXX_V	  	:= $(CROSS_COMPILE_0)g++
AR_V      	:= $(CROSS_COMPILE_0)ar
CP_V      	:= $(CROSS_COMPILE_0)objcopy
OBJDUMP_V 	:= $(CROSS_COMPILE_0)objdump 
SIZE_V 	  	:= $(CROSS_COMPILE_0)size
JTAGPROG_V  := openocd
RM_V		:= rm
LD_V		:= $(CROSS_COMPILE_0)g++
 
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

#FPU GCC settings
MCU_SFPU_FLAGS := -mfloat-abi=hard -mfpu=fpv4-sp-d16 -ffast-math -fsingle-precision-constant
ifeq ($(MCU_MAJOR_TYPE),f4)
MCU_FLAGS := -mcpu=cortex-m4 $(MCU_SFPU_FLAGS) -DSTM32MCU_MAJOR_TYPE_F4
endif
ifeq ($(MCU_MAJOR_TYPE),f37)
MCU_FLAGS := -mcpu=cortex-m4 $(MCU_SFPU_FLAGS) -DSTM32MCU_MAJOR_TYPE_F37
endif
ifeq ($(MCU_MAJOR_TYPE),f2)
MCU_FLAGS := -mcpu=cortex-m3 -DSTM32MCU_MAJOR_TYPE_F2
endif
ifeq ($(MCU_MAJOR_TYPE),f1)
MCU_FLAGS := -mcpu=cortex-m3 -DSTM32MCU_MAJOR_TYPE_F1
endif
ifeq ($(MCU_FLAGS),)
$(error MCU_MAJOR_TYPE CPU type is invalid)
endif


CXXFLAGS += -std=gnu++14
ASFLAGS += -Wa,-mapcs-32 -mcpu=$(MCU) -mthumb $(COMMON_FLAGS)
LDFLAGS +=  -L$(SCRIPTS_DIR) -nostdlib -nostartfiles -T$(LSCRIPT) -mthumb
CPFLAGS =  -S
ARFLAGS = rcs
#Other compiler flags
COMMON_FLAGS += -O$(OPT) $(MCU_FLAGS) -mthumb -Wno-variadic-macros -Wno-long-long -pipe

LINK_LIBS = -Wl,--start-group -lstdc++ -lc -lm -lg -lgcc -Wl,--end-group
#If cpp exceptions is suported
ifeq ($(CPP_EXCEPTIONS),y)
COMMON_FLAGS += -DCONFIG_ENABLE_EXCEPTIONS
else
CXXFLAGS += -fno-exceptions -fno-rtti
endif

#If using SBL bootloader is required
ifeq ($(USE_SBL_BOOTLOADER),y)
COMMON_FLAGS+=-DCONFIG_WITH_SBL_BOOTLOADER_ENABLED
endif

ifeq ($(LISTING),y)
ASLST = -Wa,-adhlns=$(<:%.S=%.lst)
CLST = -Wa,-adhlns=$(<:%.c=%.lst)
CPPLST = -Wa,-adhlns=$(<:%.cpp=%.lst)
LSSTARGET = $(TARGET).lss
LDFLAGS += -Wl,-Map=$(TARGET).map,--cref 
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
CFLAGS += -ffunction-sections -fdata-sections -flto
CXXFLAGS += -ffunction-sections -fdata-sections -flto
LDFLAGS+= -Wl,--gc-sections -flto
endif
CXXFLAGS+= $(COMMON_FLAGS)
CFLAGS+= $(COMMON_FLAGS)
CFLAGS += -std=gnu11

#Assume default programmer
PGM_TYPE?=BF30
PGM_CMDLINE_CFG:='set ISIX_INTERFACE $(PGM_TYPE); 
PGM_CMDLINE_CFG+=set ISIX_INTERFACE_TARGET stm32$(MCU_MAJOR_TYPE)x; 
ifeq ($(PGM_SWD),y)
PGM_CMDLINE_CFG+=set ISIX_INTERFACE_SWD 1; 
endif
ifdef PGM_SPEED_KHZ
PGM_CMDLINE_CFG+=set ISIX_INTERFACE_SPEED_KHZ $(PGM_SPEED_KHZ); 
endif
PGM_CMDLINE_CFG+='


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

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
CC_VFX := 'python2 $(ROOT_DIR)/cc_args.py $(CC_V)'
CXX_VFX := 'python2 $(ROOT_DIR)/cc_args.py $(CXX_V)'

.PHONY : vimfiles
.ONESHELL: vimfiles
vimfiles:
	@rm -f .clang_complete
	@$(MAKE) -s clean all CC=$(CC_VFX) CXX=$(CXX_VFX)
	@echo | $(CC_V) -Wp,-v -x c++ - -fsyntax-only 2>&1 | awk ' /^ / { print "-isystem\n"$$1 >> ".clang_complete"; } END { print "-std=c++14" >> ".clang_complete"; } '
	@awk 'BEGIN { v="."; }  /^-I/ { gsub("-I","", $$1); v=v","$$1} END { print "set path="v  > ".vim.custom"; }' .clang_complete
	@echo "let lb_grep_path='$(realpath $(ISIX_BASE_DIR)/..)'" >> .vim.custom


program: $(TARGET).elf
	$(JTAGPROG) -c $(PGM_CMDLINE_CFG) -f $(SCRIPTS_DIR)/stm32.cfg -c "program $(TARGET).elf verify reset"  -c shutdown

.PHONY : devrst
devrst:
	$(JTAGPROG) -c $(PGM_CMDLINE_CFG) -f $(SCRIPTS_DIR)/stm32.cfg -c init -c reset run -c shutdown

.PHONY : devdbg
devdbg:
	@echo "openocd -c $(PGM_CMDLINE_CFG) -f $(realpath $(SCRIPTS_DIR)/stm32.cfg)"

ifeq ($(LIBRARY),y)
build:	target  
else
build:	target $(LSSTARGET) size-calc
endif


ifeq ($(LIBRARY),y)
target:	lib$(TARGET).a
else
target: $(TARGET).elf $(TARGET).$(FORMAT) $(LSSTARGET)
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

