#4 Automatic makefile for GNUARM (C/C++)

#Target binary file name
TARGET	   = unittest

#Output format (elf,hex,bin)
FORMAT	= bin

#Optimalization [0,1,2,3,s]
# 0 - none optimalization, s - size optimalization 3 - most optimized
OPT 	?= g

#MCU variant
#MCU_MAJOR_TYPE = f4
#MCU_MINOR_TYPE = 17VGT6
#MCU_MAJOR_TYPE = f1
#MCU_MINOR_TYPE = 07VBT6
MCU_MAJOR_TYPE = f2
MCU_MINOR_TYPE = 17VG

#MCU_MAJOR_TYPE = f37
#MCU_MINOR_TYPE = 3xCT6

#System stack size interrupts etc
MCU_SYSTEM_STACK_SIZE = 1024

#Enable exceptions support
CPP_EXCEPTIONS = y

#Common flags 
COMMON_FLAGS = -Wall -Wextra  -Wno-variadic-macros  
COMMON_FLAGS+= -D_HAVE_CONFIG_H -I./include -DFUNCTION_MAIN_RETURN

#Include
#only for f1 lib
#COMMON_FLAGS +=-DSTM32F10X_CL

#Use SWD programmer
PGM_SWD = n
PGM_SPEED_KHZ=2000


#C compiler options
CFLAGS += $(COMMON_FLAGS)

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

ISIX_BASE=../..
ISIX_DIR=$(ISIX_BASE)/libisix
ISIXDRV_DIR=$(ISIX_BASE)/libisixdrvstm32
LIBFOUNDATION_DIR=$(ISIX_BASE)/libfoundation
LIBSTM32_DIR=$(ISIX_BASE)/libstm32
include $(ISIX_DIR)/isix.mk
include $(ISIXDRV_DIR)/isixdrv.mk
include $(LIBFOUNDATION_DIR)/libfoundation.mk
include $(LIBSTM32_DIR)/libstm32.mk
include $(LIBSTM32_DIR)/scripts/stm32.mk

