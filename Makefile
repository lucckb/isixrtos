# Automatic makefile for ARM compiler (C/C++) 

#tutaj wpisz nazwe pliku hex
TARGET	   = isix

#Optymalizacja [0,1,2,3,s]
# 0 - brak optymalizacji, s -optymalizacja rozmiaru
OPT 	?= 2

#Default isix architecture
ISIX_ARCH ?= arm-cm3

#Opcje kompilatora C
CFLAGS += -std=gnu99 -Wall -pedantic

CFLAGS += -Iinclude -I../libfoundation/include -Iarch/$(ISIX_ARCH)/include

#Wlaczyc wylaczyc listing
LISTING ?= n

#Wlaczyc wylaczyc debug
DEBUG ?=  y


#Kernel source 
SRC += kernel/fifo.c  kernel/memory.c  kernel/scheduler.c  kernel/semaphore.c  kernel/task.c kernel/cppsupport.c kernel/irqtimers.c

#Architecture specific sources
SRC += arch/$(ISIX_ARCH)/port_scheduler.c

#pliki assemblerowe
ASRC +=

include isix-deprecated.mk

