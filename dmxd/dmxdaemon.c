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
#include <sys/time.h>
#include <time.h>
#include <artnet/artnet.h>
#include <artnet/packets.h>
#include "dmxdaemon.h"

/* File descriptors */
static int sock;
static int artnet_fd;
static int dmx_out_fd;

static int operation_id = 1;
static int transaction_id = 1;
static int fullcontrol = 0;

static unsigned char artnet_dmx[512];
static unsigned char internal_dmx[512];

static artnet_node node ;
static char *artnet_ip = NULL;
static int verbose;
static int simulate = 0;

static int artnet_pps = 0;
static int dmxd_pps = 0;
static int dmx_pps = 0;

#define OUTPUT_PPS 44.2

#define MAX_CONNECTIONS 20
#define MAX_OPERATIONS (512*4)
#define BUFLEN 1024

static struct dmxd_connection connections[MAX_CONNECTIONS];
static struct dmxd_operation operations[MAX_OPERATIONS];

#define FUNC_FADE 1
#define FUNC_BLINK 2
#define FUNC_CONTROL 3
#define FUNC_LOCK 4
#define FUNC_MAX 6
#define FUNC_MIN 7
#define FUNC_ADD 8
#define FUNC_SUB 9
#define FUNC_CANCEL 10
#define FUNC_CANCELTRANS 11
#define FUNC_SCALEMAX 12
#define FUNC_SCALEMIN 13
#define FUNC_TRANSACTION_START 14
#define FUNC_TRANSACTION_END 15

#define RET_COMMAND 0
#define RET_TRANSACTION 1

#define ENTTEC_SEND 6

static int dmxd_read_byte(struct dmxd_operation *op, int pos, unsigned char *out) {
	if (op != NULL) {
		unsigned char variable;
		memcpy(&variable, op->arguments + pos, sizeof(unsigned char));
		*out = variable;
		return sizeof(unsigned char);
	}
}

static int dmxd_read_short(struct dmxd_operation *op, int pos, unsigned short *out) {
	if (op != NULL) {
		unsigned short variable;
		memcpy(&variable, op->arguments + pos, sizeof(unsigned short));
		*out = ntohs(variable);
		return sizeof(unsigned short);
	}
}

static int dmxd_read_int(struct dmxd_operation *op, int pos, int *out) {
	if (op != NULL) {
		int variable;
		memcpy(&variable, op->arguments + pos, sizeof(int));
		*out = ntohl(variable);
		return sizeof(int);
	}
}

static int dmxd_read_float(struct dmxd_operation *op, int pos, float *out) {
	if (op != NULL) {
		float variable;
		memcpy(&variable, op->arguments + pos, sizeof(float));
		*out = variable;
		return sizeof(float);
	}
}

static int dmxd_send_udp(struct dmxd_operation *op, unsigned char result, int value) {
	unsigned char buffer[1024];
	unsigned short uniqueid = op->uniqueid;
	
	uniqueid = htons(uniqueid);
	value = htonl(value);
	
	memcpy(buffer, &uniqueid, 2);
	memcpy(buffer + 2, &result, 1);
	memcpy(buffer + 3, &value, 4);
	
	sendto(sock, buffer, 7, MSG_DONTWAIT, (const struct sockaddr*)connections[op->connection_id].from, sizeof(connections[op->connection_id].from));
}

static int dmxd_transmit_dmx() {
	unsigned char dmx[517];
	int length = 512;
	unsigned char lLength = length & 0xFF;
	unsigned char hLength = (length >> 8) & 0xFF;
	int count=0;

	dmx[0] = 0x7E;
	dmx[1] = ENTTEC_SEND;
	dmx[2] = lLength;
	dmx[3] = hLength;
	memcpy(dmx+4, internal_dmx, length);
	dmx[length+4] = 0xE7;

	count = write(dmx_out_fd, dmx, length+5);
	
	dmx_pps++;
}

static int dmxd_output_console() {
	int i;
	printf("\e[H\e[2J");
	for (i = 0; i < 512; i++) {
		printf("%02X ", internal_dmx[i]);
		if (i % 30 == 29)
			printf("\n");
	}
	printf("\n");
}

static void dmxd_set_dmx(short channel, unsigned char value) {
	internal_dmx[channel] = value;
}

static struct dmxd_operation *parse_packet(unsigned char *data, int length) {
	static struct dmxd_operation operation;
	if (length < 4)
		return NULL;

	if (length - 4 > DMXD_MAX_ARG_LEN)
		return NULL;

	memset(&operation, 0, sizeof(struct dmxd_operation));
	operation.transaction_id = -1;
	memcpy(&operation.uniqueid, data, sizeof(operation.uniqueid));
	operation.uniqueid = ntohs(operation.uniqueid);
	operation.command = data[2];
	operation.flags = data[3];
	memcpy(operation.arguments, data + 4, length - 4);
	operation.argument_len = length - 4;

	if (verbose)
	printf("Parsed command: %d, uniqueid: %d\n", operation.command, operation.uniqueid);

	dmxd_pps++;

	return &operation;
}

static void operation_remove(struct dmxd_operation *op) {
	if (verbose)
	printf("Removing oid: %d\n", op->operation_id);
	if (op != NULL && op->operation_id >= 0) {
		operations[op->operation_id].active = 0;
		operations[op->operation_id].allocated = 0;
	}
}

static void operation_activate(struct dmxd_operation *op) {
	if (verbose)
	printf("Activating oid: %d, (%d)\n", op->operation_id, op->command);
	if (op != NULL) {
		operations[op->operation_id].active = 1;
		gettimeofday(&operations[op->operation_id].start_time, NULL);
	}
}

/* Checks if a channel is already in use, and if it should be overriden
 * The return value says if the operation should continue or not
 */
static int should_override(struct dmxd_operation*op) {
	int i;
	if (!op->has_run) {
		op->has_run = 1;

		if (op->flags & DMXD_OP_OVERRIDE) {
			for (i = 0; i < MAX_OPERATIONS; ++i) {
				if (i != op->operation_id && operations[i].active && operations[i].channel == op->channel) {
					operation_remove(&operations[i]);
				}
			}
			return 1;
		} else {
			for (i = 0; i < MAX_OPERATIONS; ++i) {
				if (i != op->operation_id && operations[i].active && operations[i].channel == op->channel) {
					/* abort current command */
					return 0;
				}
			}
			return 1;
		}
	} else {
		return 1;
	}
}

inline static int enough_arguments(const char *function_name, struct dmxd_operation *op, int size) {
	if (op->argument_len < size) {
		fprintf(stderr, "%s: Too small packet\n", function_name);
		operation_remove(op);
		return 0;
	}
	return 1;
}

static void f_fade(struct dmxd_operation *op, float runtime) {
	int len = 0;
	short channel;
	unsigned char fromval;
	unsigned char toval;
	float timespan;

	if (!enough_arguments(__func__, op, 8))
		return;

	len += dmxd_read_short(op, len, &channel);
	len += dmxd_read_byte(op, len, &fromval);
	len += dmxd_read_byte(op, len, &toval);
	len += dmxd_read_float(op, len, &timespan);

	op->channel = channel;

	/* Cancel operation if no override flag, cancel all others if override flag */
	if (!should_override(op)) {
		operation_remove(op);
		return;
	}

	if (verbose)
		printf("f%d: %02d %f\n", op->operation_id, fromval + (int)((float)((toval-fromval) * (float)(runtime/timespan))), runtime);

	dmxd_set_dmx(channel, min(fromval + (int)((float)((toval-fromval) * (float)(runtime/timespan))), toval));
	
	if (runtime >= timespan) {
		operation_remove(op);
	}
}

static void f_blink(struct dmxd_operation *op, float runtime) {
	int len = 0;
	short channel;
	unsigned char lowval;
	unsigned char highval;
	float timeup;
	float timedown;
	int times;
	
	if (!enough_arguments(__func__, op, 16))
		return;
	
	len += dmxd_read_short(op, len, &channel);
	len += dmxd_read_byte(op, len, &lowval);
	len += dmxd_read_byte(op, len, &highval);
	len += dmxd_read_float(op, len, &timeup);
	len += dmxd_read_float(op, len, &timedown);
	len += dmxd_read_int(op, len, &times);

	op->channel = channel;

	/* Cancel operation if no override flag, cancel all others if override flag */
	if (!should_override(op)) {
		operation_remove(op);
		return;
	}

	float totaltime = timeup + timedown;
	int times_run = runtime / totaltime;
	float current_time = runtime - (times_run * totaltime);

	if (times_run == times) {
		operation_remove(op);
		return;
	}
	
	if (current_time < timedown) {
		if (verbose)
			printf("b%d: %02d %f r%d/%d\n", op->operation_id, highval, runtime, times_run, times);
		dmxd_set_dmx(channel, highval);
	} else {
		if (verbose)
			printf("b%d: %02d %f r%d/%d\n", op->operation_id, lowval, runtime, times_run, times);
		dmxd_set_dmx(channel, lowval);
	}
}

static void f_control(struct dmxd_operation *op, float runtime) {
	int len = 0;
	short channel;
	float timespan;

	/* Cancel operation if no override flag, cancel all others if override flag */
	if (!should_override(op)) {
		operation_remove(op);
		return;
	}

	/* stop passing artnet data the next frame */
	fullcontrol = 1;

	if (runtime >= 1) {
		operation_remove(op);
	}
}

static void f_scalemax(struct dmxd_operation *op, float runtime) {
	int len = 0;
	short channel;
	unsigned char maxval;
	float forsecs;

	if (!enough_arguments(__func__, op, 7))
		return;

	len += dmxd_read_short(op, len, &channel);
	len += dmxd_read_byte(op, len, &maxval);
	len += dmxd_read_float(op, len, &forsecs);

	op->channel = channel;

	/* Cancel operation if no override flag, cancel all others if override flag */
	if (!should_override(op)) {
		operation_remove(op);
		return;
	}
	
	unsigned char value = (unsigned char)((float)(artnet_dmx[channel] / 255.f) * maxval);

	dmxd_set_dmx(channel, value);

	if (verbose)
		printf("sma%d: %02d %f\n", op->operation_id, value, runtime);

	if (runtime >= forsecs) {
		operation_remove(op);
	}
}

static void f_scalemin(struct dmxd_operation *op, float runtime) {
	int len = 0;
	short channel;
	unsigned char minval;
	float forsecs;

	if (!enough_arguments(__func__, op, 7))
		return;

	len += dmxd_read_short(op, len, &channel);
	len += dmxd_read_byte(op, len, &minval);
	len += dmxd_read_float(op, len, &forsecs);

	op->channel = channel;

	/* Cancel operation if no override flag, cancel all others if override flag */
	if (!should_override(op)) {
		operation_remove(op);
		return;
	}
	
	unsigned char value = minval + (unsigned char)((float)(artnet_dmx[channel] / 255.f) * (255-minval));

	dmxd_set_dmx(channel, value);

	if (verbose)
		printf("smi%d: %02d %f\n", op->operation_id, value, runtime);

	if (runtime >= forsecs) {
		operation_remove(op);
	}
}

static void f_lock(struct dmxd_operation *op, float runtime) {
	int len = 0;
	short channel;
	unsigned char value;
	float timespan;

	if (!enough_arguments(__func__, op, 7))
		return;

	len += dmxd_read_short(op, len, &channel);
	len += dmxd_read_byte(op, len, &value);
	len += dmxd_read_float(op, len, &timespan);

	op->channel = channel;

	/* Cancel operation if no override flag, cancel all others if override flag */
	if (!should_override(op)) {
		operation_remove(op);
		return;
	}

	dmxd_set_dmx(channel, value);
	if (verbose)
		printf("l%d: %02d %f\n", op->operation_id, value, runtime);
	if (runtime >= timespan) {
		operation_remove(op);
	}
}

static void f_max(struct dmxd_operation *op, float runtime) {
	int len = 0;
	short channel;
	unsigned char maxval;
	float forsecs;

	if (!enough_arguments(__func__, op, 7))
		return;

	len += dmxd_read_short(op, len, &channel);
	len += dmxd_read_byte(op, len, &maxval);
	len += dmxd_read_float(op, len, &forsecs);

	op->channel = channel;

	/* Cancel operation if no override flag, cancel all others if override flag */
	if (!should_override(op)) {
		operation_remove(op);
		return;
	}

	unsigned char value = min(artnet_dmx[channel],maxval);

	dmxd_set_dmx(channel, value);

	if (verbose)
		printf("ma%d: %02d %f\n", op->operation_id, value, runtime);

	if (runtime >= forsecs) {
		operation_remove(op);
	}
}

static void f_min(struct dmxd_operation *op, float runtime) {
	int len = 0;
	short channel;
	unsigned char minval;
	float forsecs;

	if (!enough_arguments(__func__, op, 7))
		return;

	len += dmxd_read_short(op, len, &channel);
	len += dmxd_read_byte(op, len, &minval);
	len += dmxd_read_float(op, len, &forsecs);

	op->channel = channel;

	/* Cancel operation if no override flag, cancel all others if override flag */
	if (!should_override(op)) {
		operation_remove(op);
		return;
	}

	unsigned char value = max(artnet_dmx[channel],minval);

	dmxd_set_dmx(channel, value);

	if (verbose)
		printf("mi%d: %02d %f\n", op->operation_id, value, runtime);

	if (runtime >= forsecs) {
		operation_remove(op);
	}
}

static void f_add(struct dmxd_operation *op, float runtime) {
	int len = 0;
	short channel;
	unsigned char addval;
	float forsecs;

	if (!enough_arguments(__func__, op, 7))
		return;

	len += dmxd_read_short(op, len, &channel);
	len += dmxd_read_byte(op, len, &addval);
	len += dmxd_read_float(op, len, &forsecs);

	op->channel = channel;

	/* Cancel operation if no override flag, cancel all others if override flag */
	if (!should_override(op)) {
		operation_remove(op);
		return;
	}

	unsigned char value = min(artnet_dmx[channel] + addval,255);

	dmxd_set_dmx(channel, value);

	if (verbose)
		printf("ad%d: %02d %f\n", op->operation_id, value, runtime);

	if (runtime >= forsecs) {
		operation_remove(op);
	}
}

static void f_sub(struct dmxd_operation *op, float runtime) {
	int len = 0;
	short channel;
	unsigned char subval;
	float forsecs;

	if (!enough_arguments(__func__, op, 7))
		return;

	len += dmxd_read_short(op, len, &channel);
	len += dmxd_read_byte(op, len, &subval);
	len += dmxd_read_float(op, len, &forsecs);

	op->channel = channel;

	/* Cancel operation if no override flag, cancel all others if override flag */
	if (!should_override(op)) {
		operation_remove(op);
		return;
	}

	unsigned char value = max((int)artnet_dmx[channel] - subval,0);

	dmxd_set_dmx(channel, value);

	if (verbose)
		printf("ad%d: %02d %f\n", op->operation_id, value, runtime);

	if (runtime >= forsecs) {
		operation_remove(op);
	}
}

static void f_cancel(struct dmxd_operation *op, float runtime) {
	int len = 0;
	int operation_id;
	int i;

	if (!enough_arguments(__func__, op, 4))
		return;

	dmxd_read_int(op, len, &operation_id);

	/* Cancel operation if no override flag, cancel all others if override flag */
	if (!should_override(op)) {
		operation_remove(op);
		return;
	}

	for (i = 0; i < MAX_OPERATIONS; ++i) {
		if (operation_id == 0 || (operation_id == i && operations[i].allocated == 1)) {
			operation_remove(&operations[i]);
		}
	}

	operation_remove(op);
}

static void f_cancel_trans(struct dmxd_operation *op, float runtime) {
	int len = 0;
	int transaction_id;
	int i;

	if (!enough_arguments(__func__, op, 4))
		return;

	dmxd_read_int(op, len, &transaction_id);

	/* Cancel operation if no override flag, cancel all others if override flag */
	if (!should_override(op)) {
		operation_remove(op);
		return;
	}

	for (i = 0; i < MAX_OPERATIONS; ++i) {
		if (transaction_id == 0 || (transaction_id == operations[i].transaction_id && operations[i].allocated == 1)) {
			operation_remove(&operations[i]);
		}
	}

	operation_remove(op);
}


static void f_transaction_end(struct dmxd_operation *op, float runtime) {
	if (verbose)
		printf("Ran transaction end function\n");
	if (connections[op->connection_id].transaction_id != -1) {
		int i;
		for (i = 0; i < MAX_OPERATIONS; ++i) {
			if (operations[i].allocated && 
				operations[i].active == 0 && 
				operations[i].transaction_id == op->transaction_id) {
					if (verbose)
						printf("Activating transacted operation %d (%d)\n", operations[i].operation_id, operations[i].command);
					operation_activate(&operations[i]);
			}
		}
		operations[op->operation_id].transaction_id = -1;
		connections[op->connection_id].transaction_id = -1;
		op->transaction_id = -1;
	}
	if (runtime >= 0)
		operation_remove(op);
}

static void f_transaction_start(struct dmxd_operation *op, float runtime) {
	unsigned char buffer[100];
	short n_uniqueid = htons(op->uniqueid);
	int n_transaction_id = htonl(transaction_id);
	int bufferlen;

	/* Force end last transaction if you start a new */
	if (connections[op->connection_id].transaction_id > -1) {
		f_transaction_end(op, -1);
	}
	
	if (verbose) {
		printf("Ran transaction start function\n");
		printf("Assigned transaction id %d\n", transaction_id);
	}
	connections[op->connection_id].transaction_id = transaction_id;

	dmxd_send_udp(op, RET_TRANSACTION, transaction_id);
	if (verbose)
		printf("Sent packet back about transaction_id=%d\n", transaction_id);

	transaction_id++;
	operation_remove(op);
}

static void *functions[] = {
	(void *)FUNC_FADE, (void *)f_fade,
	(void *)FUNC_LOCK, (void *)f_lock,
	(void *)FUNC_MAX, (void *)f_max,
	(void *)FUNC_MIN, (void *)f_min,
	(void *)FUNC_ADD, (void *)f_add,
	(void *)FUNC_SUB, (void *)f_sub,
	(void *)FUNC_CANCEL, (void *)f_cancel,
	(void *)FUNC_CANCELTRANS, (void *)f_cancel_trans,
	(void *)FUNC_CONTROL, (void *)f_control,
	(void *)FUNC_BLINK, (void *)f_blink,
	(void *)FUNC_SCALEMAX, (void *)f_scalemax,
	(void *)FUNC_SCALEMIN, (void *)f_scalemin,
	(void *)FUNC_TRANSACTION_START, (void *)f_transaction_start,
	(void *)FUNC_TRANSACTION_END, (void *)f_transaction_end,
};

static void (*find_function(unsigned char func)) (struct dmxd_operation *op, float runtime) {
	int i;
	
	for (i = 0; i < sizeof(functions); i += 2) {
		if ((int)functions[i] == func)
			return functions[i+1];
	}
	return NULL;
}

static int find_connection(struct sockaddr_in *addr) {
	int i;
	for (i = 0; i < MAX_CONNECTIONS; ++i) {
		if (connections[i].inuse && memcmp((void *)connections[i].from, (void *)addr, sizeof(addr)) == 0) {
			if (verbose)
				printf("Connection found.\n");
			return i;
		}
	}
	return -1;
}

static int add_connection(struct sockaddr_in *addr) {
	int i;
	for (i = 0; i < MAX_CONNECTIONS ; ++i) {
		if (connections[i].inuse == 0) {
			connections[i].inuse = 1;
			connections[i].from = addr;
			connections[i].transaction_id = -1;
			if (verbose)
				printf("Connection added.\n");
			return i;
		}
	}
	fprintf(stderr, "Tried to accept new connection, but no more connectionslots available\n");
	return -1;
}

/* TODO: Use a 'ever'-increasing operation_id, but still check if operations are in use.
 * or else a 'start_transaction' and 'end_transaction' could come out of order in some seldom cases
 */
static int add_operation(struct dmxd_operation *operation) {
	int i;
	
	for (i = 0; i < MAX_OPERATIONS; ++i) {
		if (operations[i].allocated == 0) {
			operation->allocated = 1;
			operation->operation_id = i;
			memcpy(&operations[i], operation, sizeof(struct dmxd_operation));
			if (verbose)
				printf("Operation added with command: %d, Active: %d, Operation-id: %d\n", operation->command, operations[i].active, i);
			
			dmxd_send_udp(operation, RET_COMMAND, i);
			return i;
		}
	}
	fprintf(stderr, "Tried to add operation to queue, but no more operationslots available\n");
	return -1;
}

static void handle_operations(void) {
	int i;
	int todo = 0;
	
	for (i = 0; i < MAX_OPERATIONS; ++i) {
		if (operations[i].allocated && operations[i].active) {
			void (*func)(struct dmxd_operation *op, float runtime);
			todo++;
			if (verbose)
				printf("Handling operation %d (%d)\n", operations[i].operation_id, operations[i].command);
			func = find_function(operations[i].command);
			if (func != NULL) {
				float duration;
				struct timeval now;
				gettimeofday(&now, NULL);
				duration = (now.tv_sec - operations[i].start_time.tv_sec) + ((now.tv_usec / 1000000.f) - (operations[i].start_time.tv_usec / 1000000.f));
				func(&operations[i], duration);
			} else {
				/* Invalid function, remove */
				fprintf(stderr,"Received command %d unknown\n", operations[i].command);
				operation_remove(&operations[i]);
			}
		}
	}
	if (verbose && todo > 0)
	  printf("Handeled %d jobs\n", todo);
}

int dmx_callback(artnet_node n, void *p, void *d) {
	static time_t last = 0;
	static int counter = 0;
	time_t now;
	static int last_seq = 0;

	artnet_packet pack = (artnet_packet) p;

	// first time
	if(last ==0) {
		time(&last);
		last_seq = pack->data.admx.sequence;
	}

	memcpy(artnet_dmx, (unsigned char *)&(pack->data.admx.data), 512);

	time(&now) ;

	if(verbose && pack->data.admx.sequence - last_seq > 1) {
			printf("lost %d packets %d %d \n", pack->data.admx.sequence - last_seq, pack->data.admx.sequence , last_seq );
	}
	
	if(last == now) 
		counter++ ;
	else {
		if (verbose)
			printf("Got %d packets last second\n", counter) ;
		counter = 0;
		last = now ;
	}
	last_seq = pack->data.admx.sequence ;

	artnet_pps++;
	
	return 0;
}

int init_artnet() {
	node = artnet_new(artnet_ip, verbose);

	artnet_set_short_name(node, "Artnet -> DMX ");
	artnet_set_long_name(node, "ArtNet to EntTec DMX adapter. (c) Raider 2011");
	artnet_set_node_type(node, ARTNET_NODE);

	artnet_set_handler(node, ARTNET_DMX_HANDLER, dmx_callback, NULL);

	artnet_set_port_type(node, 0, ARTNET_ENABLE_OUTPUT, ARTNET_PORT_DMX);
	artnet_set_subnet_addr(node, 0);

	artnet_set_port_addr(node, 0, ARTNET_OUTPUT_PORT, 0) ;
	artnet_start(node);

	artnet_fd = artnet_get_sd(node);
}

inline void copy_artnet_to_dmx() {
	if (!fullcontrol)
		memcpy(internal_dmx, artnet_dmx, 512);
}

int main(int argc, char **argv) {
	struct sockaddr_in si_me, si_other;
	int len, slen = sizeof(si_other);
	unsigned char buffer[BUFLEN];
	fd_set selectlist;
	int highsock;
	int optc;
	struct timeval updated;
	char usb_dflt_device[] = "/dev/ttyUSB0";
	char *usb_device = usb_dflt_device;
	
	struct timeval timeout;
	
	// parse options 
	while ((optc = getopt (argc, argv, "a:d:vs")) != EOF) {
		switch (optc) {
			case 'a':
				artnet_ip = (char *) strdup(optarg);
				break;

			case 'd':
				usb_device = (char *) strdup(optarg);
				break;
			case 's':
				simulate = 1;
				break;
			case 'v':
				verbose = 1;
				break;

		}
	}

	if (argc == 0 || artnet_ip == NULL) {
		fprintf(stderr, "Usage: %s -a <artnet_ip> [-d /dev/ttyUSBx] [-v]\n", argv[0]);
		return 1;
	}

	if (!simulate) {
		dmx_out_fd = open(usb_device, O_RDWR | O_NOCTTY | O_NONBLOCK);
		if (dmx_out_fd <= 0) {
			fprintf(stderr, "Could not open USB device %s.\n", usb_device);
			return 1;
		}
	}

	init_artnet();

	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("dmxd socket");
		return 1;
	}

	memset((void *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(DMXD_UDP_PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (const struct sockaddr*)&si_me, sizeof(si_me)) == -1) {
		perror("dmxd udp port");
		close(sock);
		return 1;
	}

	if (artnet_fd > sock)
		highsock = artnet_fd;
	else
		highsock = sock;

	/* save cursor */
	printf("\e7");
	
	/* Mainloop */
	while (1) {
		static struct timeval lastsend;
		struct timeval now;
		timeout.tv_sec = 0;
		timeout.tv_usec = 1000000 / OUTPUT_PPS;

		FD_ZERO(&selectlist);
		FD_SET(sock, &selectlist);
		FD_SET(artnet_fd, &selectlist);

		gettimeofday(&now, NULL);
		long long diff = (long long)(now.tv_sec - lastsend.tv_sec) * 1000000;
		diff += now.tv_usec - lastsend.tv_usec;

		timeout.tv_usec -= diff;

		int ready = select(highsock + 1, &selectlist, 0, 0, &timeout);

		if (ready > 0) {
			if (FD_ISSET(sock, &selectlist)) {
				len = recvfrom(sock, buffer, BUFLEN, 0, (struct sockaddr *)&si_other, &slen);
				if (len == -1) {
					perror("recvfrom");
					return 1;
				}
				if (verbose)
					printf("DEBUG: %d bytes of data received\n", len);
				int connection_id = find_connection(&si_other);
				if (connection_id == -1) {
					if (connection_id = add_connection(&si_other) == -1) {
						/* No more connections available, ignore packet */
						continue;
					}
				}
				struct dmxd_operation* operation = parse_packet(buffer, len);
				if (operation != NULL) {
					operation->connection_id = connection_id;
					operation->transaction_id = connections[connection_id].transaction_id;

					if (verbose)
						printf("Connection_id = %d, transaction_id = %d\n", operation->connection_id, operation->transaction_id);

					add_operation(operation);

					/* No transaction, run immediately */
					if (operation->transaction_id == -1) {
						operation_activate(operation);
					} else
					/* Transaction start/end commands has to be run immediately anyways */
					if (operation->command == FUNC_TRANSACTION_START || operation->command == FUNC_TRANSACTION_END) {
						operation_activate(operation);
						/* TODO: Handle transactions directly, so next commands gets correct transaction id */
					}
				} else {
					printf("Invalid packet received\n");
				}
			}
			if (FD_ISSET(artnet_fd, &selectlist)) {
				if (verbose)
					printf("artnet read\n");
				artnet_read(node, 0);
			}
		}

		gettimeofday(&now, NULL);
		diff = (long long)(now.tv_sec - lastsend.tv_sec) * 1000000;
		diff += now.tv_usec - lastsend.tv_usec;
		if (diff < (1000000 / OUTPUT_PPS))
			continue;

		/* Come here at least every *timeout* uS */
		copy_artnet_to_dmx();
		/* Reset for each frame */
		fullcontrol = 0;
		handle_operations();
		gettimeofday(&lastsend, NULL);
		if (!simulate)
			dmxd_transmit_dmx();
		else
			dmxd_output_console();
		
		if (!verbose && !simulate && (now.tv_sec > updated.tv_sec)) {
			int i,num_operations = 0;
			for (i = 0; i < MAX_OPERATIONS; ++i) {
				if (operations[i].allocated)
					num_operations++;
			}
			printf("\e8\e[2KCurrent operations: %d Artnet pps: %d DMXd pps: %d DMX Out pps: %d ", num_operations, artnet_pps, dmxd_pps, dmx_pps);
			fflush(stdout);
			dmxd_pps = 0;
			artnet_pps = 0;
			dmx_pps = 0;
			gettimeofday(&updated, NULL);
		}
	}
}
