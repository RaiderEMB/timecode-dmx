#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <alsa/asoundlib.h>
#include <alsa/asoundef.h>
#include <alsa/global.h>
#include <alsa/rawmidi.h>
#include <linux/input.h>

static volatile int stop;
int current_note, previous_note;
snd_rawmidi_t *input;
snd_rawmidi_t *output;

unsigned char packet_signature[] = { 0x13, 0x37 };

#define UDPMIDI_PORT 1205

static void sig_handler(int signal)
{
	stop = 1;
}

int main(int argc, char **argv) {
        int inputf, e, err;
	int sock;
        struct input_event buffer;
	int npfds;
	struct pollfd *pfds;
	struct sockaddr_in si_me, si_other;
	int len, slen = sizeof(si_other);
	int optval = 1;

        if(argc < 2) {
                fprintf(stderr, "%s <device>\n", argv[0]);
                exit(-1);
        }
        e = snd_rawmidi_open(&input, &output, argv[1], SND_RAWMIDI_NONBLOCK);//O_WRONLY);
	if (e == -1) {
	        fprintf(stderr, "snd:rawmidi:open-%X\n", e);
		return 1;
	}

	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("udp socket");
		return 1;
	}

	memset((void *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(UDPMIDI_PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	memset((void *) &si_other, 0, sizeof(si_me));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(UDPMIDI_PORT);
	si_other.sin_addr.s_addr = htonl(INADDR_BROADCAST);

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

	npfds = snd_rawmidi_poll_descriptors_count(input) + 1;
	pfds = alloca((npfds + 1) * sizeof(struct pollfd));
	snd_rawmidi_poll_descriptors(input, pfds, npfds);
	pfds[npfds-1].fd = sock;
	pfds[npfds-1].events = POLLRDNORM;

        while(1) {
		int i,rlen, midiin;
		err = poll(pfds, npfds, 200);

		midiin = 0;
		for (i = 0; i < npfds; ++i) {
			if (pfds[i].revents & POLLIN)
				midiin = 1;
		}
		if (midiin) {
			unsigned char buf[10];
			unsigned char outbuf[12];

			printf("got midi\n");
			rlen = snd_rawmidi_read(input, buf, sizeof(buf));
			if (rlen == -EAGAIN)
				continue;
			if (rlen < 0) {
				fprintf(stderr, "Cannot read from port \"%s\": %s", argv[1], snd_strerror(err));
				break;
			}
			// UDP broadcast
			memcpy(outbuf, packet_signature, 2);
			if (rlen > 10) continue;
			memcpy(outbuf + 2, buf, rlen);
			if (sendto(sock, outbuf, 2 + rlen, 0, (struct sockaddr *)&si_other, sizeof(si_other)) == -1) {
				fprintf(stderr, "Error sending broadcast packet.\n");
			}
		}

		if (pfds[npfds-1].revents & POLLRDNORM) {
			int len,slen;
			unsigned char buffer[10];

			printf("Got udp\n");
			slen = sizeof(si_other);
			len = recvfrom(sock, buffer, 10, 0, (struct sockaddr *)&si_other, &slen);
			if (len > 2 && memcmp(buffer, packet_signature, 2) == 0) {
        			int wlen = snd_rawmidi_write(output, buffer + 2, len - 2);
				if (wlen < len - 2) {
					fprintf(stderr, "Midi out: Wrote %d of %d bytes\n", wlen, len - 2);
				}
			}
		}
        }

        return 0;
}
