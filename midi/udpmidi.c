#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
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

void play() {
        char buffer[512] = {0};
        int sz;

        /* Notes: 90 on, 80 off, B0 cc, C0 pc, E0 bend */
        buffer[0] = 0xB0;
        buffer[1] = 00;
        buffer[2] = 00;
        sz = snd_rawmidi_write(output, buffer, 3);
	sleep(1);
        printf("(off)snd:rawmidi:write-%X\n", sz);
        buffer[0] = 0xB0;
        buffer[1] = 00;
        buffer[2] = 0x70;
        sz = snd_rawmidi_write(output, buffer, 3);
	sleep(1);
        printf("(on)snd:rawmidi:write-%X\n", sz);
}

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

        if(argc < 2) {
                fprintf(stderr, "%s /dev/input/event<X>\n", argv[0]);
                exit(-1);
        }
        e = snd_rawmidi_open(&input, &output, argv[1], SND_RAWMIDI_NONBLOCK);//O_WRONLY);
	if (e == -1) {
	        printf("snd:rawmidi:open-%X\n", e);
	}

	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("udp socket");
		return 1;
	}

	memset((void *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(1005);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (const struct sockaddr*)&si_me, sizeof(si_me)) == -1) {
		perror("udp port");
		close(sock);
		return 1;
	}

	npfds = snd_rawmidi_poll_descriptors_count(input);
	pfds = alloca((npfds + 1) * sizeof(struct pollfd));
	snd_rawmidi_poll_descriptors(input, pfds, npfds);
	pfds[npfds].fd = sock;
	pfds[npfds].events = POLLIN;

        while(1) {
		int i,rlen, midiin;
		err = poll(pfds, npfds, 200);

		midiin = 0;
		for (i = 0; i < npfds; ++i) {
			if (pdfs[i].revents & POLLIN)
				midiin = 1;
		}
		if (midiin) {
			rlen = snd_rawmidi_read(input, buf, sizeof(buf));
			if (rlen == -EAGAIN)
				continue;
			if (rlen < 0) {
				fprintf(stderr, "Cannot read from port \"%s\": %s", argv[1], snd_strerror(err));
				break;
			}
			// UDP broadcast
		}

		if (pdfs[npfds].revents & POLLIN) {
			int len,slen;
			unsigned char buffer[10]:
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
