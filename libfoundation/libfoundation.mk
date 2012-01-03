ifneq ($(ISIX_ARCH),simunix-x86)
LIBFOUNDATION_O_CPPSRC += $(LIBFOUNDATION_DIR)/syscalls.cpp
endif
LIBFOUNDATION_SRC += $(LIBFOUNDATION_DIR)/tiny_alloc.c
LIBFOUNDATION_SRC += $(LIBFOUNDATION_DIR)/tiny_printf.c
LIBFOUNDATION_SRC += $(LIBFOUNDATION_DIR)/utils.c
LIBFOUNDATION_CPPSRC += $(LIBFOUNDATION_DIR)/virtual_eeprom.cpp
LIBFOUNDATION_INC += -I$(LIBFOUNDATION_DIR)/include
LIBFOUNDATION_LIB = $(LIBFOUNDATION_DIR)/libfoundation.a
LIBFOUNDATION_OBJS += $(LIBFOUNDATION_SRC:%.c=%.o) $(LIBFOUNDATION_CPPSRC:%.cpp=%.o)
DEPFILES += $(LIBFOUNDATION_SRC:%.c=%.dep) $(LIBFOUNDATION_CPPSRC:%.cpp=%.dep) $(LIBFOUNDATION_O_CPPSRC:%.cpp=%.dep)

$(LIBFOUNDATION_LIB): $(LIBFOUNDATION_OBJS)
	    $(AR) $(ARFLAGS) $@ $^

LIBS += $(LIBFOUNDATION_LIB)
CRT0_OBJECTS += $(LIBFOUNDATION_O_CPPSRC:%.cpp=%.o)
LIBS_OBJS += $(LIBFOUNDATION_OBJS) $(CRT0_OBJECTS)