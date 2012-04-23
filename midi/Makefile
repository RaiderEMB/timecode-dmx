all:	udpmidi midisend

udpmidi:	udpmidi.c
	gcc -o udpmidi udpmidi.c -lasound

midisend:	send.c
	gcc -o midisend send.c

clean:
	rm -f udpmidi
