#Base folder LIBGFX_DIR dir
LIBGFXSRC_DIR = $(LIBGFX_DIR)/src
LIBGFXINC_DIR = $(LIBGFX_DIR)/include
LIBGFX_DRVDISP_SRC_DIR = $(LIBGFX_DIR)/drivers/disp
LIBGFX_DRVINPUT_SRC_DIR = $(LIBGFX_DIR)/drivers/input
LIBGFXFONTSRC_DIR = $(LIBGFX_DIR)/src/fonts

LIBGFX_INC = -I$(LIBGFXINC_DIR)

LIBGFX_DRV_CPPSRC = $(LIBGFX_DRVDISP_SRC_DIR)/ili9341.cpp
LIBGFX_CPPSRC =  $(LIBGFXSRC_DIR)/gdi.cpp
LIBGFX_CPPSRC += $(LIBGFXSRC_DIR)/gui/widget.cpp
LIBGFX_CPPSRC += $(LIBGFXSRC_DIR)/gui/window.cpp
LIBGFX_CPPSRC += $(LIBGFXSRC_DIR)/gui/button.cpp
LIBGFX_CPPSRC += $(LIBGFXSRC_DIR)/gui/label.cpp
LIBGFX_CPPSRC += $(LIBGFXSRC_DIR)/gui/frame.cpp
LIBGFX_CPPSRC += $(LIBGFXSRC_DIR)/input.cpp
LIBGFX_CPPSRC += $(LIBGFXFONTSRC_DIR)/default.cpp


LIBGFX_OBJS = $(LIBGFX_CPPSRC:%.cpp=%.o) $(LIBGFX_DRV_CPPSRC:%.cpp=%.o) 
LIBGFX_LIB =  $(LIBGFX_DIR)/libgfx.a
DEPFILES +=   $(LIBGFX_CPPSRC:%.cpp=%.dep) $(LIBGFX_DRV_CPPSRC:%.cpp=%.dep)

.ONESHELL:
$(LIBGFX_LIB): $(LIBGFX_OBJS)
	    $(AR) $(ARFLAGS) $@ $^

LIBS += $(LIBGFX_LIB)
LIBS_OBJS += $(LIBGFX_OBJS)
COMMON_FLAGS += $(LIBGFX_INC)
