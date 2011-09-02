all: dmxdaemon udptest

dmxdaemon: dmxdaemon.c dmxdaemon.h
	gcc -g -o dmxdaemon -lm -lartnet dmxdaemon.c

udptest: udptest.c
	gcc -o udptest udptest.c
