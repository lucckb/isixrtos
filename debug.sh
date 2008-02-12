echo "reset halt" > /tmp/pgm.script
rxvt -e openocd -f lpc2148.cfg &
sleep 2
rxvt -e arm-elf-gdbtui isix.elf &
 