#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include "showdaemon.h"

struct showd_packet {
	unsigned short length;
	char data[1024];
};

static int showd_packet_init(struct showd_packet *packet, enum show_commands command) {
	printf("Preparing packet for command %d\n", command);

	// Size unknown yet
	packet->data[0] = 0;
	packet->data[1] = 0;
	packet->length  = 2;

	packet->data[2] = command;
	packet->length++;

	return packet->length;
}

static int showd_packet_add_char(struct showd_packet *packet, unsigned char data) {
	packet->data[packet->length] = data;
	return ++packet->length;
}

static int showd_packet_add_short(struct showd_packet *packet, unsigned short data) {
	unsigned char *p = packet->data;
	data = htons(data);

	memcpy(p + packet->length, &data, sizeof(unsigned short));
	packet->length += sizeof(unsigned short);
	return packet->length;
}

static int showd_packet_add_int(struct showd_packet *packet, unsigned int data) {
	data = htonl(data);
	
	memcpy(&(packet->data[packet->length]), &data, sizeof(unsigned int));
	packet->length += sizeof(unsigned int);
	return packet->length;
}

static int showd_packet_add_float(struct showd_packet *packet, float data) {
	memcpy(&(packet->data[packet->length]), &data, sizeof(float));
	packet->length += sizeof(float);
	return packet->length;
}

static int showd_packet_save(struct showd_packet *packet, FILE *fp) {
	unsigned short len = htons(packet->length);
	size_t wrote;

	memcpy(&(packet->data), &len, sizeof(unsigned short));
	wrote = fwrite(&(packet->data), packet->length + 2, 1, fp);

	if (wrote != 1) {
		fprintf(stderr, "Error writing command %d to file: %s\n", packet->data[2], strerror(errno));
	}

	return wrote;
}

int main(int argc, char **argv) {
	FILE *fp;
	struct showd_packet packet;

	if ((fp = fopen(argv[1],"w")) <= 0) {
		perror("fopen");
		return 1;
	}
	fwrite("showdaemon", 11, 1, fp);

	showd_packet_init(&packet, SHOW_FUNC_EFFECT_START);
	showd_packet_add_short(&packet, 1);
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_LOCK);
	showd_packet_add_char(&packet, 1); // step 1
	showd_packet_add_short(&packet, 10); // channel 10
	showd_packet_add_char(&packet, 255); // to 255
	showd_packet_add_float(&packet, 4); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_FADE);
	showd_packet_add_char(&packet, 1); // step 1
	showd_packet_add_short(&packet, 10); // channel 10
	showd_packet_add_char(&packet, 0); // from 0
	showd_packet_add_char(&packet, 255); // to 255
	showd_packet_add_float(&packet, 3); // 3 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_FADE);
	showd_packet_add_char(&packet, 1); // step 1
	showd_packet_add_short(&packet, 10); // channel 10
	showd_packet_add_char(&packet, 0); // from 0
	showd_packet_add_char(&packet, 255); // to 255
	showd_packet_add_float(&packet, 5); // 5 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_FADE);
	showd_packet_add_char(&packet, 2); // step 2
	showd_packet_add_short(&packet, 10); // channel 10
	showd_packet_add_char(&packet, 0); // from 0
	showd_packet_add_char(&packet, 255); // to 255
	showd_packet_add_float(&packet, 2); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_EFFECT_END);
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_EFFECT_START);
	showd_packet_add_short(&packet, 2);
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_FADE);
	showd_packet_add_char(&packet, 1); // step 1
	showd_packet_add_short(&packet, 10); // channel 10
	showd_packet_add_char(&packet, 0); // from 0
	showd_packet_add_char(&packet, 255); // to 255
	showd_packet_add_float(&packet, 0.5); // 5 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_FADE);
	showd_packet_add_char(&packet, 1); // step 2
	showd_packet_add_short(&packet, 10); // channel 10
	showd_packet_add_char(&packet, 0); // from 0
	showd_packet_add_char(&packet, 255); // to 255
	showd_packet_add_float(&packet, 0.6); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_FADE);
	showd_packet_add_char(&packet, 2); // step 2
	showd_packet_add_short(&packet, 10); // channel 10
	showd_packet_add_char(&packet, 0); // from 0
	showd_packet_add_char(&packet, 255); // to 255
	showd_packet_add_float(&packet, 0.4); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_EFFECT_END);
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_TS_START);
	showd_packet_add_int(&packet, 38481);
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_LOCK);
	showd_packet_add_short(&packet, 1); // channel 10
	showd_packet_add_char(&packet, 15); // from 0
	showd_packet_add_float(&packet, 4); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_LOCK);
	showd_packet_add_short(&packet, 2); // channel 10
	showd_packet_add_char(&packet, 13); // from 0
	showd_packet_add_float(&packet, 2); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_LOCK);
	showd_packet_add_short(&packet, 3); // channel 10
	showd_packet_add_char(&packet, 0); // from 0
	showd_packet_add_float(&packet, 5); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_FADE);
	showd_packet_add_short(&packet, 4); // channel 10
	showd_packet_add_char(&packet, 255); // from 0
	showd_packet_add_char(&packet, 0); // to 255
	showd_packet_add_float(&packet, 1); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_FADE);
	showd_packet_add_short(&packet, 5); // channel 10
	showd_packet_add_char(&packet, 255); // from 0
	showd_packet_add_char(&packet, 0); // to 255
	showd_packet_add_float(&packet, 1); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_TS_END);
	showd_packet_save(&packet, fp);

	//
	showd_packet_init(&packet, SHOW_FUNC_TS_START);
	showd_packet_add_int(&packet, 40154);
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_LOCK);
	showd_packet_add_short(&packet, 2); // channel 10
	showd_packet_add_char(&packet, 12); // from 0
	showd_packet_add_float(&packet, 0.15); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_EFFECT);
	showd_packet_add_short(&packet, 1); // effect 1
	showd_packet_add_short(&packet, 2); // channel 2
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_EFFECT);
	showd_packet_add_short(&packet, 1); // effect 1
	showd_packet_add_short(&packet, 3); // channel 2
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_EFFECT);
	showd_packet_add_short(&packet, 2); // effect 1
	showd_packet_add_short(&packet, 4); // channel 4
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_TS_END);
	showd_packet_save(&packet, fp);

	//
	showd_packet_init(&packet, SHOW_FUNC_TS_START);
	showd_packet_add_int(&packet, 40636);
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_LOCK);
	showd_packet_add_short(&packet, 2); // channel 10
	showd_packet_add_char(&packet, 12); // from 0
	showd_packet_add_float(&packet, 0.15); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_TS_END);
	showd_packet_save(&packet, fp);

	//
	showd_packet_init(&packet, SHOW_FUNC_TS_START);
	showd_packet_add_int(&packet, 41138);
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_LOCK);
	showd_packet_add_short(&packet, 2); // channel 10
	showd_packet_add_char(&packet, 12); // from 0
	showd_packet_add_float(&packet, 0.4); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_TS_END);
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_TS_START);
	showd_packet_add_int(&packet, 40362);
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_LOCK);
	showd_packet_add_short(&packet, 1); // channel 10
	showd_packet_add_char(&packet, 15); // from 0
	showd_packet_add_float(&packet, 2); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_BLINK);
	showd_packet_add_short(&packet, 2); // channel 10
	showd_packet_add_char(&packet, 0); // from 0
	showd_packet_add_char(&packet, 13); // to 255
	showd_packet_add_float(&packet, 1); // 4 seconds
	showd_packet_add_float(&packet, 1); // 4 seconds
	showd_packet_add_int(&packet, 1);
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_LOCK);
	showd_packet_add_short(&packet, 3); // channel 10
	showd_packet_add_char(&packet, 0); // from 0
	showd_packet_add_float(&packet, 2); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_FADE);
	showd_packet_add_short(&packet, 4); // channel 10
	showd_packet_add_char(&packet, 0); // from 0
	showd_packet_add_char(&packet, 255); // to 255
	showd_packet_add_float(&packet, 2); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_FADE);
	showd_packet_add_short(&packet, 5); // channel 10
	showd_packet_add_char(&packet, 0); // from 0
	showd_packet_add_char(&packet, 255); // to 255
	showd_packet_add_float(&packet, 2); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_TS_END);
	showd_packet_save(&packet, fp);

	//

	showd_packet_init(&packet, SHOW_FUNC_TS_START);
	showd_packet_add_int(&packet, 43662);
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_LOCK);
	showd_packet_add_short(&packet, 1); // channel 10
	showd_packet_add_char(&packet, 15); // from 0
	showd_packet_add_float(&packet, 2); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_BLINK);
	showd_packet_add_short(&packet, 2); // channel 10
	showd_packet_add_char(&packet, 0); // from 0
	showd_packet_add_char(&packet, 13); // to 255
	showd_packet_add_float(&packet, 1); // 4 seconds
	showd_packet_add_float(&packet, 1); // 4 seconds
	showd_packet_add_int(&packet, 1);
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_LOCK);
	showd_packet_add_short(&packet, 3); // channel 10
	showd_packet_add_char(&packet, 40); // from 0
	showd_packet_add_float(&packet, 2); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_FADE);
	showd_packet_add_short(&packet, 4); // channel 10
	showd_packet_add_char(&packet, 0); // from 0
	showd_packet_add_char(&packet, 255); // to 255
	showd_packet_add_float(&packet, 2); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_FADE);
	showd_packet_add_short(&packet, 5); // channel 10
	showd_packet_add_char(&packet, 0); // from 0
	showd_packet_add_char(&packet, 255); // to 255
	showd_packet_add_float(&packet, 2); // 4 seconds
	showd_packet_save(&packet, fp);

	showd_packet_init(&packet, SHOW_FUNC_TS_END);
	showd_packet_save(&packet, fp);

	//
	
	return 0;
}