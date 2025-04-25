

sudo chmod 777 /dev/ttyUSB0 && make -j16 && make flash && picocom /dev/ttyUSB0 -b 115200 