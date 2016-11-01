
LIBSTM32_O_SRC += $(LIBSTM32_DIR)/src/isr_bootvect.c
LIBSTM32_SRC += $(LIBSTM32_DIR)/src/usart_simple.c
LIBSTM32_SRC += $(LIBSTM32_DIR)/src/crashinfo.c
LIBSTM32_SRC += $(LIBSTM32_DIR)/src/mpu_v7m.c
LIBSTM32_SRC += $(LIBSTM32_DIR)/src/crt0.c
#Only F1 type support regular memory controller for flash emu
ifeq ($(MCU_MAJOR_TYPE),f1)
LIBSTM32_CPPSRC += $(LIBSTM32_DIR)/src/stm32f1fmc.cpp
endif
LIBSTM32_INC += -I$(LIBSTM32_DIR)/inc -DCPP_STARTUP_CODE

LIBSTM32_LIB = $(LIBSTM32_DIR)/libstm32lite.a
LIBSTM32_OBJS += $(LIBSTM32_SRC:%.c=%.o) $(LIBSTM32_CPPSRC:%.cpp=%.o)
DEPFILES += $(LIBSTM32_SRC:%.c=%.dep) $(LIBSTM32_CPPSRC:%.cpp=%.dep)  $(LIBSTM32_O_SRC:%.c=%.dep)

.ONESHELL:
$(LIBSTM32_LIB): $(LIBSTM32_OBJS)
	$(AR) $(ARFLAGS) $@ $^

LIBS += $(LIBSTM32_LIB)
CRT0_OBJECTS += $(LIBSTM32_O_SRC:%.c=%.o)
LIBS_OBJS += $(LIBSTM32_OBJS) $(CRT0_OBJECTS)
COMMON_FLAGS += $(LIBSTM32_INC)
