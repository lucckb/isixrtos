# Automatic makefile for GNUARM (C/C++)

#tutaj wpisz nazwe pliku hex
TARGET	   = isixdrv

#Format wyjsciowy (moze byc elf,hex,bin)
FORMAT	= bin

#Optymalizacja [0,1,2,3,s]
# 0 - brak optymalizacji, s -optymalizacja rozmiaru
OPT 	?= 2


#Common flags 
COMMON_FLAGS = -Wall -I./include

#Opcje kompilatora C
CFLAGS += $(COMMON_FLAGS)
CFLAGS += -std=gnu99 

#Opcje kompilatora C++
CXXFLAGS += $(COMMON_FLAGS)


#Wlaczyc wylaczyc listing
LISTING = n

#Wlaczyc wylaczyc debug
DEBUG ?= n

#Czy biblioteka
LIBRARY = y

#pliki zrodlowe C
SRC += $(wildcard *.c)

#pliki zrodlowe C++
CPPSRC += $(wildcard *.cpp)

#pliki assemblerowe
ASRC += $(wildcard *.S)



include ../lib-stm32/scripts/stm32.mk

