echo "reset halt" > /tmp/pgm.script
konsole -e openocd -f lpc2148.cfg & 2> /dev/null

