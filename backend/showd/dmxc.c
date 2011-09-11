#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "dmxc.h"

static int dmxc_sock;
struct sockaddr_in si_dmxd;

int dmxc_udp_send(struct dmxc_packet *packet) {
	return sendto(dmxc_sock, packet->data, packet->length, MSG_DONTWAIT, (const struct sockaddr*)&si_dmxd, sizeof(si_dmxd));
}

int dmxc_packet_init(struct dmxc_packet *packet, enum dmxd_commands command, char flag) {
	unsigned short nuniqueid;
	packet->uniqueid = rand() % 65536;

	//printf("Preparing packet for command %d, uniqueid %d\n", command, packet->uniqueid);
	nuniqueid = htons(packet->uniqueid);
	memcpy(packet->data, &nuniqueid, sizeof(unsigned short));
	packet->length = 2;
	
	packet->data[packet->length] = command;
	packet->length++;
	packet->data[packet->length] = flag;
	packet->length++;

	return packet->length;
}

static int dmxc_packet_add_char_v(struct dmxc_packet *packet, va_list *args) {
	unsigned char data = va_arg(*args, int);

	packet->data[packet->length] = data;
	return ++packet->length;
}

static int dmxc_packet_add_short_v(struct dmxc_packet *packet, va_list *args) {
	unsigned short data = va_arg(*args, int);
	unsigned char *p = packet->data;
	printf("packet_add(short): %d\n", data);
	data = htons(data);

	memcpy(p + packet->length, &data, sizeof(unsigned short));
	packet->length += sizeof(unsigned short);
	return packet->length;
}

static int dmxc_packet_add_int_v(struct dmxc_packet *packet, va_list *args) {
	unsigned int data = va_arg(*args, int);
	data = htonl(data);
	
	memcpy(&(packet->data[packet->length]), &data, sizeof(unsigned int));
	packet->length += sizeof(unsigned int);
	return packet->length;
}

static int dmxc_packet_add_float_v(struct dmxc_packet *packet, va_list *args) {
	float data = va_arg(*args, double);
	
	memcpy(&(packet->data[packet->length]), &data, sizeof(float));
	packet->length += sizeof(float);
	return packet->length;
}

int dmxc_packet_add_char(struct dmxc_packet *packet, unsigned char data) {
	packet->data[packet->length] = data;
	return ++packet->length;
}

int dmxc_packet_add_short(struct dmxc_packet *packet, unsigned short data) {
	unsigned char *p = packet->data;
	printf("packet_add(short): %d\n", data);
	data = htons(data);

	memcpy(p + packet->length, &data, sizeof(unsigned short));
	packet->length += sizeof(unsigned short);
	return packet->length;
}

int dmxc_packet_add_int(struct dmxc_packet *packet, unsigned int data) {
	data = htonl(data);
	
	memcpy(&(packet->data[packet->length]), &data, sizeof(unsigned int));
	packet->length += sizeof(unsigned int);
	return packet->length;
}

int dmxc_packet_add_float(struct dmxc_packet *packet, float data) {
	memcpy(&(packet->data[packet->length]), &data, sizeof(float));
	packet->length += sizeof(float);
	return packet->length;
}


static int dmxc_sendv(enum dmxd_commands command, char flag, va_list args) {
	int len = 0;
	struct dmxc_packet packet;
	dmxc_packet_init(&packet, command, flag);

	switch (command) {
		case DMXD_FUNC_FADE:
			dmxc_packet_add_short_v(&packet, &args); /* channel */
			dmxc_packet_add_char_v(&packet, &args);  /* fromval */
			dmxc_packet_add_char_v(&packet, &args);  /* toval   */
			dmxc_packet_add_float_v(&packet, &args); /* seconds */

			len = dmxc_udp_send(&packet);
			break;

		case DMXD_FUNC_BLINK:
			dmxc_packet_add_short_v(&packet, &args); /* channel */
			dmxc_packet_add_char_v(&packet, &args);  /* lowval  */
			dmxc_packet_add_char_v(&packet, &args);  /* highval */
			dmxc_packet_add_float_v(&packet, &args); /* secdown */
			dmxc_packet_add_float_v(&packet, &args); /* secup   */
			dmxc_packet_add_int_v(&packet, &args);   /* times   */

			len = dmxc_udp_send(&packet);
			break;

		case DMXD_FUNC_CONTROL:
			len = dmxc_udp_send(&packet);
			break;

		case DMXD_FUNC_LOCK:
			dmxc_packet_add_short_v(&packet, &args); /* channel */
			dmxc_packet_add_char_v(&packet, &args);  /* value   */
			dmxc_packet_add_float_v(&packet, &args); /* seconds */
			len = dmxc_udp_send(&packet);
			break;

		case DMXD_FUNC_MAX:
			dmxc_packet_add_short_v(&packet, &args); /* channel */
			dmxc_packet_add_char_v(&packet, &args);  /* maxval  */
			dmxc_packet_add_float_v(&packet, &args); /* seconds */
			len = dmxc_udp_send(&packet);
			break;

		case DMXD_FUNC_MIN:
			dmxc_packet_add_short_v(&packet, &args); /* channel */
			dmxc_packet_add_char_v(&packet, &args);  /* minval  */
			dmxc_packet_add_float_v(&packet, &args); /* seconds */
			len = dmxc_udp_send(&packet);
			break;

		case DMXD_FUNC_ADD:
			dmxc_packet_add_short_v(&packet, &args); /* channel */
			dmxc_packet_add_char_v(&packet, &args);  /* addval  */
			dmxc_packet_add_float_v(&packet, &args); /* seconds */
			len = dmxc_udp_send(&packet);
			break;

		case DMXD_FUNC_SUB:
			dmxc_packet_add_short_v(&packet, &args); /* channel */
			dmxc_packet_add_char_v(&packet, &args);  /* subval  */
			dmxc_packet_add_float_v(&packet, &args); /* seconds */
			len = dmxc_udp_send(&packet);
			break;

		case DMXD_FUNC_SCALEMAX:
			dmxc_packet_add_short_v(&packet, &args); /* channel */
			dmxc_packet_add_char_v(&packet, &args);  /* maxval  */
			dmxc_packet_add_float_v(&packet, &args); /* seconds */
			len = dmxc_udp_send(&packet);
			break;

		case DMXD_FUNC_SCALEMIN:
			dmxc_packet_add_short_v(&packet, &args); /* channel */
			dmxc_packet_add_char_v(&packet, &args);  /* minval  */
			dmxc_packet_add_float_v(&packet, &args); /* seconds */
			len = dmxc_udp_send(&packet);
			break;

		case DMXD_FUNC_TRANSACTION_START:
			len = dmxc_udp_send(&packet);
			break;

		case DMXD_FUNC_TRANSACTION_END:
			len = dmxc_udp_send(&packet);
			break;

		default:
			fprintf(stderr, "%s: Unknown command %d\n", __func__, command);
	}
	if (len) {
		printf("Packet size: %d Data sent: %d\n", packet.length, len);
	}

}

int dmxc_init(char* hostname, short unsigned int port) {
	srand(time(NULL));
	struct hostent *hostbyname = gethostbyname(hostname);
        if(hostbyname == NULL) {
                fprintf(stderr, "Could not resolve %s", hostname);
		perror(__func__);
                exit(1);
        }

	if ((dmxc_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("dmxc_init: socket");
		exit(1);
	}

	memset((void *) &si_dmxd, 0, sizeof(si_dmxd));
	si_dmxd.sin_family = AF_INET;
	si_dmxd.sin_port = htons(port);
	memcpy(&si_dmxd.sin_addr.s_addr, hostbyname->h_addr, sizeof(si_dmxd.sin_addr.s_addr));
}

int dmxc_send(enum dmxd_commands command, ... ) {
	va_list argp;
	va_start(argp, command);
	dmxc_sendv(command, 1, argp);
	va_end(argp);
}

int dmxc_sendf(enum dmxd_commands command, char flag, ... ) {
	va_list argp;
	va_start(argp, flag);
	dmxc_sendv(command, flag, argp);
	va_end(argp);
}

