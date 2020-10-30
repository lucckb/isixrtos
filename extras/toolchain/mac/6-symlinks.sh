#!/bin/bash
for i in /usr/local/arm-none-eabi-gcc/*; do
	ln -s $i /usr/local/bin
done
