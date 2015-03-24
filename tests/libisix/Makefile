#4 Automatic makefile for GNUARM (C/C++)

#Target binary file name
TARGET	   = unittest

#Output format (elf,hex,bin)
FORMAT	= elf

#Optimalization [0,1,2,3,s]
# 0 - none optimalization, s - size optimalization 3 - most optimized
OPT 	?= 2

#MCU variant
#MCU_MAJOR_TYPE = f4
#MCU_MINOR_TYPE = 17VGT6
#MCU_MAJOR_TYPE = f1
#MCU_MINOR_TYPE = 07VBT6
MCU_MAJOR_TYPE = f2
MCU_MINOR_TYPE = 17VG

#System stack size interrupts etc
MCU_SYSTEM_STACK_SIZE = 1024

#Enable exceptions support
CPP_EXCEPTIONS = y

#Common flags 
COMMON_FLAGS = -Wall -pedantic -Wextra  -Wno-variadic-macros  
COMMON_FLAGS+= -D_HAVE_CONFIG_H -I./include -DFUNCTION_MAIN_RETURN

#Include
#only for f1 lib
#COMMON_FLAGS +=-DSTM32F10X_CL

#C compiler options
CFLAGS += $(COMMON_FLAGS)
CFLAGS += -std=gnu99 

#C++ compiler options 
CXXFLAGS += $(COMMON_FLAGS)


#Per file listing
LISTING = n

#Debug version
DEBUG ?=  y


#Source C files
SRC += $(wildcard *.c) 

#Source C++ files
CPPSRC += $(wildcard *.cpp)

#Source ASM files
ASRC += $(wildcard *.S)

ISIX_DIR=../isixrtos/libisix
ISIXDRV_DIR=../isixrtos/libisixdrvstm32
LIBFOUNDATION_DIR=../isixrtos/libfoundation
LIBSTM32_DIR=../isixrtos/libstm32
include $(ISIX_DIR)/isix.mk
include $(ISIXDRV_DIR)/isixdrv.mk
include $(LIBFOUNDATION_DIR)/libfoundation.mk
include $(LIBSTM32_DIR)/libstm32.mk
include $(LIBSTM32_DIR)/scripts/stm32.mk

