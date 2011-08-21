#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "dmxc.h"
#include <artnet/packets.h>

struct dmxc_packet {
	unsigned short uniqueid;
	int length;
	char data[1024];
};



static int dmxc_packet_init(struct dmxc_packet *packet, enum dmxd_commands command, char flag) {
	unsigned short nuniqueid;
	packet->uniqueid = rand() * 65536;

	nuniqueid = htons(packet->uniqueid);
	memcpy(packet->data, &nuniqueid, sizeof(unsigned short));
	packet->length = 2;
	
	packet->data[packet->length] = command;
	packet->length++;
	packet->data[packet->length] = flag;
	packet->length++;

	return packet->length;
}

static int dmxc_packet_add_char(struct dmxc_packet *packet, va_list args) {
	unsigned char data = va_arg(args, int);
	
	packet->data[packet->length] = data;
	return ++packet->length;
}

static int dmxc_packet_add_short(struct dmxc_packet *packet, va_list args) {
	unsigned short data = va_arg(args, int);
	data = htons(data);
	
	memcpy(&(packet->data[packet->length]), &data, sizeof(unsigned short));
	packet->length += sizeof(unsigned short);
	return packet->length;
}

static int dmxc_packet_add_int(struct dmxc_packet *packet, va_list args) {
	unsigned int data = va_arg(args, int);
	data = htonl(data);
	
	memcpy(&(packet->data[packet->length]), &data, sizeof(unsigned int));
	packet->length += sizeof(unsigned int);
	return packet->length;
}

static int dmxc_packet_add_float(struct dmxc_packet *packet, va_list args) {
	float data = va_arg(args, double);
	
	memcpy(&(packet->data[packet->length]), &data, sizeof(float));
	packet->length += sizeof(float);
	return packet->length;
}


static int dmxc_sendv(enum dmxd_commands command, char flag, va_list args) {
	struct dmxc_packet packet;
	dmxc_packet_init(&packet, command, flag);

	switch (command) {
		case FUNC_FADE:
		{
			dmxc_packet_add_short(&packet, args); /* channel */
			dmxc_packet_add_char(&packet, args);  /* fromval */
			dmxc_packet_add_char(&packet, args);  /* toval */
			dmxc_packet_add_float(&packet, args);  /* seconds */
			
			printf("Packet size: %d\n", packet.length);
		}
		break;
	}

}

int dmxc_init(char* hostname, short unsigned int port) {

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

