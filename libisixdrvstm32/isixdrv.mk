
ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/adc_converter.cpp
ISIXDRV_CPPSRC += $(ISIXDRV_DIR)/usart_buffered.cpp
ISIXDRV_SRC    += $(ISIXDRV_DIR)/ethernetif.c

ISIXDRV_INC += -I$(ISIXDRV_DIR)/include

ISIXDRV_LIB = $(ISIXDRV_DIR)/libisixdrv-stm32.a
ISIXDRV_OBJS += $(ISIXDRV_SRC:%.c=%.o) $(ISIXDRV_CPPSRC:%.cpp=%.o)
DEPFILES += $(ISIXDRV_SRC:%.c=%.dep) $(ISIXDRV_CPPSRC:%.cpp=%.dep)

.ONESHELL:
$(ISIXDRV_LIB): $(ISIXDRV_OBJS)
	    $(AR) $(ARFLAGS) $@ $^

LIBS += $(ISIXDRV_LIB)
LIBS_OBJS += $(ISIXDRV_OBJS)


