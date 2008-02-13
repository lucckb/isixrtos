echo "reset halt" > /tmp/pgm.script
konsole -e openocd -f lpc2148.cfg &
arm-elf-gdbtui isix.elf 
