OTGDRV_DIR 			= $(LIBUSBSTM32_DIR)/otgdriver
OTGDRVSRC_DIR 		= $(OTGDRV_DIR)/src
USBDEVICE_DIR 		= $(LIBUSBSTM32_DIR)/usbdev
USBDEVCORESRC_DIR 	= $(USBDEVICE_DIR)/Core/src
LIBUSBCLASS_DIR 	= $(USBDEVICE_DIR)/Class
LIBUSBISIXDRV_DIR   = $(LIBUSBSTM32_DIR)/isixdrv

#OTG driver source files
OTGDRIVER_SRC = $(OTGDRVSRC_DIR)/usb_core.c $(OTGDRVSRC_DIR)/usb_dcd_int.c $(OTGDRVSRC_DIR)/usb_dcd.c \
			    $(OTGDRVSRC_DIR)/usb_hcd_int.c $(OTGDRVSRC_DIR)/usb_hcd.c


USBDEVICECORE_SRC = $(USBDEVCORESRC_DIR)/usbd_core.c $(USBDEVCORESRC_DIR)/usbd_ioreq.c $(USBDEVCORESRC_DIR)/usbd_req.c

LIBUSBBOARD_SRC = $(LIBUSBSTM32_DIR)/usbbsp.c 
LIBUSBCLASS_SRC = $(LIBUSBCLASS_DIR)/cdc/src/usbd_cdc_core.c
LIBUSBISIXDRV_SRC = $(LIBUSBISIXDRV_DIR)/usbdevserial.c


#LIBUSB src files
LIBUSBSTM32_SRC = $(OTGDRIVER_SRC) $(USBDEVICECORE_SRC) $(LIBUSBBOARD_SRC) $(LIBUSBCLASS_SRC) $(LIBUSBISIXDRV_SRC)
#LIBUSB includes
LIBUSBSTM32_INC = -I$(OTGDRV_DIR)/inc -I$(USBDEVICE_DIR)/Core/inc -I$(LIBUSBCLASS_DIR)/cdc/inc/ -I$(LIBUSBISIXDRV_DIR)/include

LIBUSBSTM32_LIB = $(LIBUSBSTM32_DIR)/libusbstm32.a

LIBUSBSTM32_OBJS += $(LIBUSBSTM32_SRC:%.c=%.o) $(LIBUSBSTM32_CPPSRC:%.cpp=%.o)
DEPFILES += $(LIBUSBSTM32_SRC:%.c=%.dep) $(LIBUSBSTM32_CPPSRC:%.cpp=%.dep)

.ONESHELL:
$(LIBUSBSTM32_LIB): $(LIBUSBSTM32_OBJS)
	    $(AR) $(ARFLAGS) $@ $^


LIBS += $(LIBUSBSTM32_LIB)
LIBS_OBJS += $(LIBUSBSTM32_OBJS)
COMMON_FLAGS += $(LIBUSBSTM32_INC)
