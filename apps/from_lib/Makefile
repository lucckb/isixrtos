# Automatic makefile for GNUARM (C/C++)

#tutaj wpisz nazwe pliku hex
TARGET	   = sem_test

#Format wyjsciowy (moze byc elf,hex,bin)
FORMAT	= hex

#Optymalizacja [0,1,2,3,s]
# 0 - brak optymalizacji, s -optymalizacja rozmiaru
OPT 	= s


#Common flags 
COMMON_FLAGS = -Wall 

#Opcje kompilatora C
CFLAGS += $(COMMON_FLAGS)
CFLAGS += -std=gnu99 

#Opcje kompilatora C++
CXXFLAGS += $(COMMON_FLAGS)

#Wlaczyc wylaczyc listing
LISTING = y

#Wlaczyc wylaczyc debug
DEBUG = y


include lpc2148.mk

