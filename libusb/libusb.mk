#USB src libraries
ISIX_USBLIB_SRC += $(wildcard $(LIBUSB_DIR)/core/*.c)
ISIX_USBLIB_INC += -I$(LIBUSB_DIR)/include
#Driver layer defs
ifeq ($(ISIX_USBDRV_TYPE),stm32f2_host)
ISIX_USBLIB_SRC += $(LIBUSB_DIR)/drivers/controllers/stm32/timer.c
ISIX_USBLIB_SRC += $(LIBUSB_DIR)/drivers/controllers/stm32/usb_otg_fifo.c
ISIX_USBLIB_SRC += $(LIBUSB_DIR)/drivers/controllers/stm32/usb_otg_interrupt.c
ISIX_USBLIB_SRC += $(LIBUSB_DIR)/drivers/controllers/stm32/usbh_interrupt.c
ISIX_USBLIB_SRC += $(LIBUSB_DIR)/drivers/controllers/stm32/usbh_api.c
ISIX_USBLIB_SRC += $(LIBUSB_DIR)/drivers/controllers/stm32/usbh_configure_207.c
CRT0_OBJECTS += $(LIBUSB_DIR)/drivers/controllers/stm32/usb_otg_interrupt.o
ISIX_USBLIB_SRC += $(wildcard $(LIBUSB_DIR)/host/*.c)
ISIX_USBLIB_SRC += $(wildcard $(LIBUSB_DIR)/drivers/hostdev/*.c)
ISIX_USBLIB_INC += -DSTM32F2XX
endif
ifeq ($(ISIX_USBDRV_TYPE),stm32f1_host)
ISIX_USBLIB_SRC += $(LIBUSB_DIR)/drivers/controllers/stm32/timer.c
ISIX_USBLIB_SRC += $(LIBUSB_DIR)/drivers/controllers/stm32/usb_otg_fifo.c
ISIX_USBLIB_SRC += $(LIBUSB_DIR)/drivers/controllers/stm32/usb_otg_interrupt.c
ISIX_USBLIB_SRC += $(LIBUSB_DIR)/drivers/controllers/stm32/usbh_interrupt.c
ISIX_USBLIB_SRC += $(LIBUSB_DIR)/drivers/controllers/stm32/usbh_api.c
ISIX_USBLIB_SRC += $(LIBUSB_DIR)/drivers/controllers/stm32/usbh_configure_107.c
CRT0_OBJECTS += $(LIBUSB_DIR)/drivers/controllers/stm32/usb_otg_interrupt.o
ISIX_USBLIB_SRC += $(wildcard $(LIBUSB_DIR)/host/*.c)
ISIX_USBLIB_SRC += $(wildcard $(LIBUSB_DIR)/drivers/hostdev/*.c)
ISIX_USBLIB_INC += -DSTM32F10X_CL
endif

ISIX_USBLIB_LIB = $(LIBUSB_DIR)/libisixusb.a
ISIX_USBLIB_OBJS += $(ISIX_USBLIB_SRC:%.c=%.o) $(ISIX_USBLIB_CPPSRC:%.cpp=%.o)
DEPFILES += $(ISIX_USBLIB_SRC:%.c=%.dep) $(ISIX_USBLIB_CPPSRC:%.cpp=%.dep)

.ONESHELL:
$(ISIX_USBLIB_LIB): $(ISIX_USBLIB_OBJS)
	    $(AR) $(ARFLAGS) $@ $^

LIBS += $(ISIX_USBLIB_LIB)
LIBS_OBJS += $(ISIX_USBLIB_OBJS)
COMMON_FLAGS += $(ISIX_USBLIB_INC) 
