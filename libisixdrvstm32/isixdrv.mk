#ADC driver is deprecated only F1,F2,F4 is supported but not recommended
ifeq ($(MCU_MAJOR_TYPE),f1)
ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/adc_converter.cpp
ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/i2c_bus.cpp
endif
ifeq ($(MCU_MAJOR_TYPE),f2)
ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/adc_converter.cpp
ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/i2c_bus.cpp
endif
ifeq ($(MCU_MAJOR_TYPE),f4)
ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/adc_converter.cpp
ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/i2c_bus.cpp
endif

ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/usart_buffered.cpp
ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/stm32_spi_master.cpp
#TODO: Ethernet if for F2 and F4 platforms if needed
ifeq ($(MCU_MAJOR_TYPE),f1)
ifeq ($(ISIX_TCPIPLIB_ENABLED),y)
ISIXDRV_SRC    += $(ISIXDRV_DIR)/ethernetif.c
endif
endif
ifeq ($(MCU_MAJOR_TYPE),f2)
ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/stm32_sdio_mmc_host.cpp
endif
ifeq ($(MCU_MAJOR_TYPE),f4)
ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/stm32_sdio_mmc_host.cpp
endif
ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/mmc/mmc_card.cpp $(ISIXDRV_DIR)/mmc/mmc_slot.cpp  
ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/mmc/mmc_command.cpp $(ISIXDRV_DIR)/mmc/mmc_host_spi.cpp
ifeq ($(MCU_MAJOR_TYPE),f1)
ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/stm32_spi_master_dma.cpp
endif 
ISIXDRV_INC += -I$(ISIXDRV_DIR)/include -I$(ISIXDRV_DIR)/mmc/include
ISIXDRV_LIB = $(ISIXDRV_DIR)/libisixdrv-stm32.a
ISIXDRV_OBJS += $(ISIXDRV_SRC:%.c=%.o) $(ISIXDRV_CPPSRC:%.cpp=%.o)
DEPFILES += $(ISIXDRV_SRC:%.c=%.dep) $(ISIXDRV_CPPSRC:%.cpp=%.dep)

.ONESHELL:
$(ISIXDRV_LIB): $(ISIXDRV_OBJS)
	    $(AR) $(ARFLAGS) $@ $^

LIBS += $(ISIXDRV_LIB)
LIBS_OBJS += $(ISIXDRV_OBJS)
COMMON_FLAGS += $(ISIXDRV_INC) -DISIX_CONFIG_USE_TIMERS
