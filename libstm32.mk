
LIBSTM32_SRC += $(LIBSTM32_DIR)/src/crt0.c
LIBSTM32_SRC += $(LIBSTM32_DIR)/src/system.c
LIBSTM32_SRC += $(LIBSTM32_DIR)/src/usart_simple.c

LIBSTM32_CPPSRC += $(LIBSTM32_DIR)/src/stm32fmc.cpp

LIBSTM32_INC += -I$(LIBSTM32_DIR)/inc -DCPP_STARTUP_CODE


