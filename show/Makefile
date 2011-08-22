all: showdaemon

dmxc.o:	dmxc.c dmxc.h
	gcc -g -c -o dmxc.o dmxc.c

showdaemon: showdaemon.c showdaemon.h dmxc.o
	gcc -o showdaemon -g showdaemon.c -ljack dmxc.o
