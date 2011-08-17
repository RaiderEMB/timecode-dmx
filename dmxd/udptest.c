/***
 * DMX event daemon
 * (c) Copyright Raider, 2011
 * 
 * Author: Håkon Nessjøen <haakon.nessjoen@gmail.com>
*/
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "dmxdaemon.h"

static int sock;

/*
unsigned char test2[] = {
	12, 34, 14, 1
};
*/
/*
unsigned char test2[] = {
	12, 34, 2, 1
};
*/
unsigned char test2[] = {
	12, 34, 1, 1, 0, 123, 10, 255, 1, 1, 1, 1
};

int main() {
	float testen = 2.f;
	struct sockaddr_in si_me, si_other;
	int len, slen = sizeof(si_other);
	fd_set selectlist;
	int highsock;
	struct timeval timeout;
	unsigned char buffer[1024];
	int bufferlen;
	short uniqueid = 1337;
	
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		return 1;
	}

	memset((void *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(12345);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	memset((void *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(9118);
	si_other.sin_addr.s_addr = inet_addr("127.0.0.1");

	uniqueid = htons(uniqueid);
	memcpy(buffer, &uniqueid, sizeof(uniqueid));
	buffer[sizeof(uniqueid)] = 1;
	buffer[sizeof(uniqueid)+1] = 1;
	memcpy(buffer+4, &uniqueid, sizeof(uniqueid));

	bufferlen = 6;

testen = htonl(testen);
memcpy(test2 + 8, &testen, sizeof(testen));

	sendto(sock, test2, sizeof(test2), MSG_DONTWAIT, (const struct sockaddr*)&si_other, sizeof(si_other));
/*
	sendto(sock, buffer, bufferlen, MSG_DONTWAIT, (const struct sockaddr*)&si_other, sizeof(si_other));
*/
}
