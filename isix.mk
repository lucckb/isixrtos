# Automatic makefile for GNUARM (C/C++)

#Typ procesora
MCU	= cortex-m3


#CROSS COMPILE
CROSS_COMPILE ?= arm-none-eabi-
#Definicje programow
CC      = $(CROSS_COMPILE)gcc
CXX	= $(CROSS_COMPILE)c++
AR      = $(CROSS_COMPILE)ar
CP      = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump 
JTAGPROG  = openocd


CFLAGS += -I$(STRLIB_INC)
CXXFLAGS += -I$(STRLIB_INC)

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
endif

ifeq ($(DEBUG),y)
CFLAGS += -g -DISIX_DEBUG
CXXFLAGS += -g -DISIX_DEBUG
LDFLAGS += -g -DISIX_DEBUG
ASFLAGS += -gstabs -DISIX_DEBUG
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


all:	target


#Create object directory
$(OBJDIR):
		mkdir -p $@

target:	lib$(TARGET).a


clean:
	rm -f lib$(TARGET).a
	rm -rf $(OBJDIR)

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
.SECONDARY: lib$(TARGET).a

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


$(OBJDIR)/%.o : %.S
	@echo "Assembling..."
	$(CC) -c $(ASFLAGS) $(ASLST) $< -o $@ 


$(OBJDIR)/%.o : %.c	
	@echo "Compiling C..."
	$(CC) -c $(CFLAGS) $(CLST) $< -o $@

$(OBJDIR)/%.o : %.cpp
	@echo "Compiling C++..."
	$(CXX) -c $(CXXFLAGS) $(CPPLST) $< -o $@

%.a : $(OBJ)
	@echo "Creating library ..."
	$(AR) $(ARFLAGS) $@ $(OBJ)

