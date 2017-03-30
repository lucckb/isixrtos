ifeq ($(MCU_MAJOR_TYPE),f4)
ISIX_ARCH ?= arm-cm4f
endif
ifeq ($(MCU_MAJOR_TYPE),f1)
ISIX_ARCH ?= arm-cm3
endif
ifeq ($(MCU_MAJOR_TYPE),f2)
ISIX_ARCH ?= arm-cm3
endif
ifeq ($(MCU_MAJOR_TYPE),f3)
ISIX_ARCH ?= arm-cm4f
endif
ifeq ($(MCU_MAJOR_TYPE),f37)
ISIX_ARCH ?= arm-cm4f
endif
ifeq ($(ISIX_ARCH),)
$(error Unknown ISIX_ARCH)
endif


#Kernel source 
ISIX_SRC += $(ISIX_DIR)/kernel/fifo.c 
ISIX_SRC += $(ISIX_DIR)/kernel/memory.c 
ISIX_SRC += $(ISIX_DIR)/kernel/scheduler.c 
ISIX_SRC += $(ISIX_DIR)/kernel/semaphore.c 
ISIX_SRC += $(ISIX_DIR)/kernel/mutex.c 
ISIX_SRC += $(ISIX_DIR)/kernel/condvar.c 
ISIX_SRC += $(ISIX_DIR)/kernel/task.c 
ISIX_SRC += $(ISIX_DIR)/kernel/softtimers.c
ISIX_SRC += $(ISIX_DIR)/kernel/mempool.c
ISIX_SRC += $(ISIX_DIR)/kernel/ostime.c
ISIX_SRC += $(ISIX_DIR)/kernel/events.c
ISIX_SRC += $(ISIX_DIR)/kernel/fifo_event.c
ISIX_SRC += $(ISIX_DIR)/kernel/osstats.c
#Architecture specific sources
ISIX_SRC += $(ISIX_DIR)/arch/$(ISIX_ARCH)/port_scheduler.c
ISIX_SRC += $(ISIX_DIR)/arch/$(ISIX_ARCH)/port_memprot.c

#Isix includes
ISIX_INC += -I$(ISIX_DIR)/include -I$(ISIX_DIR)/arch/$(ISIX_ARCH)/include


ISIX_VERSION := $(shell cd $(ISIX_DIR) && git describe --tags 2> /dev/null )
ifdef ISIX_VERSION
COMMON_FLAGS += -DISIX_GIT_VERSION=\"$(ISIX_VERSION)\"
else
COMMON_FLAGS += -DISIX_GIT_VERSION=\"unknown\"
endif


ISIX_LIB = $(ISIX_DIR)/libisix.a
ISIX_OBJS += $(ISIX_SRC:%.c=%.o) 
DEPFILES += $(ISIX_SRC:%.c=%.dep) 

.ONESHELL:
$(ISIX_LIB): $(ISIX_OBJS)
	    $(AR) $(ARFLAGS) $@ $^

LIBS += $(ISIX_LIB)
LIBS_OBJS += $(ISIX_OBJS)
#Default isix configuration
COMMON_FLAGS += $(ISIX_INC) -DCOMPILED_UNDER_ISIX -DCONFIG_ISIX_USE_PREEMPTION 
