# Automatic makefile for GNUARM (C/C++)

#tutaj wpisz nazwe pliku hex
TARGET	   = isix

#Optymalizacja [0,1,2,3,s]
# 0 - brak optymalizacji, s -optymalizacja rozmiaru
OPT 	?= 2


#Opcje kompilatora C
CFLAGS += -std=gnu99 -Wall -pedantic

CFLAGS += -Iinclude

#Wlaczyc wylaczyc listing
LISTING ?= n

#Wlaczyc wylaczyc debug
DEBUG ?=  y


#Kernel source 
SRC += fifo.c  memory.c  scheduler.c  semaphore.c  task.c

#Architecture specific sorces
SRC += port_scheduler.c

#pliki assemblerowe
ASRC +=

vpath %c kernel arch/arm-cm3
include isix.mk

