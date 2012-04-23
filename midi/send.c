#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <linux/input.h>

unsigned char packet_signature[] = { 0x13, 0x37 };

#define UDPMIDI_PORT 1205

int main(int argc, char **argv) {
	int sock;
	struct sockaddr_in si_me, si_other;
	int len, slen = sizeof(si_other);
	int optval = 1;
	unsigned char buf[10],outbuf[12];

        if(argc < 2) {
                fprintf(stderr, "%s <fader value 0-127>\n", argv[0]);
                return -1;
        }

	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("udp socket");
		return 1;
	}

	memset((void *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(0);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	memset((void *) &si_other, 0, sizeof(si_me));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(UDPMIDI_PORT);
	//si_other.sin_addr.s_addr = inet_aton("127.0.0.1");//htonl(INADDR_BROADCAST);
	inet_aton("127.0.0.1", &si_other.sin_addr);

	if (bind(sock, (const struct sockaddr*)&si_me, sizeof(si_me)) == -1) {
		perror("udp port");
		close(sock);
		return 1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof (optval)) ==- 1) {
		perror("udp broadcast");
		close(sock);
		return 1;
	}

	memcpy(outbuf, packet_signature, 2);
	outbuf[2] = 0xB0;
	outbuf[3] = 0;
	outbuf[4] = atoi(argv[1]);

	if (sendto(sock, outbuf, 2 + 3, 0, (struct sockaddr *)&si_other, sizeof(si_other)) == -1) {
		fprintf(stderr, "Error sending broadcast packet.\n");
	}


        return 0;
}
