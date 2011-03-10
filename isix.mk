
#Kernel source 
ISIX_SRC += $(ISIX_DIR)/kernel/fifo.c 
ISIX_SRC += $(ISIX_DIR)/kernel/memory.c 
ISIX_SRC += $(ISIX_DIR)/kernel/scheduler.c 
ISIX_SRC += $(ISIX_DIR)/kernel/semaphore.c 
ISIX_SRC += $(ISIX_DIR)/kernel/task.c 
ISIX_SRC += $(ISIX_DIR)/kernel/cppsupport.c 
ISIX_SRC += $(ISIX_DIR)/kernel/irqtimers.c
#Architecture specific sources
ISIX_SRC += $(ISIX_DIR)/arch/$(ISIX_ARCH)/port_scheduler.c

#Isix includes
ISIX_INC += -I$(ISIX_DIR)/include -I$(ISIX_DIR)/$(ISIX_ARCH)/include

ifeq ($(DEBUG),y)
ISIX_INC += -DISIX_DEBUG
endif

