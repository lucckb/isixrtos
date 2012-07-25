ifeq ($(MCU_MAJOR_TYPE),f4)
ISIX_ARCH ?= arm-cm4f
endif
ifeq ($(MCU_MAJOR_TYPE),f1)
ISIX_ARCH ?= arm-cm3
endif
ifeq ($(MCU_MAJOR_TYPE),f2)
ISIX_ARCH ?= arm-cm3
endif

#Kernel source 
ISIX_SRC += $(ISIX_DIR)/kernel/fifo.c 
ISIX_SRC += $(ISIX_DIR)/kernel/memory.c 
ISIX_SRC += $(ISIX_DIR)/kernel/scheduler.c 
ISIX_SRC += $(ISIX_DIR)/kernel/semaphore.c 
ISIX_SRC += $(ISIX_DIR)/kernel/task.c 
ISIX_SRC += $(ISIX_DIR)/kernel/irqtimers.c
ISIX_SRC += $(ISIX_DIR)/kernel/multiple_objects.c
#Architecture specific sources
ISIX_SRC += $(ISIX_DIR)/arch/$(ISIX_ARCH)/port_scheduler.c

#Isix includes
ISIX_INC += -I$(ISIX_DIR)/include -I$(ISIX_DIR)/arch/$(ISIX_ARCH)/include

ifeq ($(DEBUG),y)
ISIX_INC += -DISIX_DEBUG
endif


ISIX_LIB = $(ISIX_DIR)/libisix.a
ISIX_OBJS += $(ISIX_SRC:%.c=%.o) 
DEPFILES += $(ISIX_SRC:%.c=%.dep) 

$(ISIX_LIB): $(ISIX_OBJS)
	    $(AR) $(ARFLAGS) $@ $^

LIBS += $(ISIX_LIB)
LIBS_OBJS += $(ISIX_OBJS)

