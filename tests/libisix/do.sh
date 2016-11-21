#!/bin/bash
~/worksrc/qemu-2.5.0-pebble4/arm-softmmu/qemu-system-arm -machine stm32-bf407 -pflash unittest.bin -serial stdio -s $1
