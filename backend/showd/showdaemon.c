/***
 * Show timecode event daemon
 * (c) Copyright Raider, 2011
 * 
 * Author: Håkon Nessjøen <haakon.nessjoen@gmail.com>
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <jack/jack.h>
#include <jack/transport.h>
#include <signal.h>
#include <errno.h>
#include "showdaemon.h"
#include "dmxc.h"

/* File descriptors */
static int sock;

jack_client_t *client;

static int verbose;

#define SHOW_UDP_PORT 9451

#define MAX_CONNECTIONS 20
#define MAX_OPERATIONS (512*4)
#define MAX_ARG_LEN 64
#define BUFLEN 1024

#define PROGRAM_NAME "showdaemon"

struct show_operation {
	char allocated;

	enum enum_operation_type {
		OP_EFFECT,
		OP_BARTRIG,
		OP_TS,
		OP_COMMAND,
	} operation_type;

	unsigned char arguments[MAX_ARG_LEN];
	int argument_len;

	enum show_commands command;

	int connection_id;
	unsigned short effect_id;
	int bartrig_id;
	int timestamp_id;
};

struct show_timestamp {
	int timestamp_id;
	int timestamp;
};

struct show_bartrig {
	int bartrig_id;
	int timestamp_start;
	int timestamp_end;
};

static int current_effect_id = -1;
static unsigned short current_bartrig_id = 0;
static unsigned short current_timestamp_id = 0;

int bartrigs_count = 0;
int timestamps_count = 0;

static struct show_operation  operations[MAX_OPERATIONS];
static struct show_timestamp  timestamps[MAX_OPERATIONS];
static struct show_bartrig    bartrigs[MAX_OPERATIONS];

void signal_handler (int sig) {
        jack_client_close (client);

        fprintf (stderr, "signal received, exiting ...\n");
        exit (0);
}

void jack_shutdown (void *arg) {
        exit (1);
}

static int show_read_byte(struct show_operation *op, int pos, unsigned char *out) {
	if (op != NULL) {
		unsigned char variable;
		memcpy(&variable, op->arguments + pos, sizeof(unsigned char));
		*out = variable;
		return sizeof(unsigned char);
	}
}

static int show_read_short(struct show_operation *op, int pos, unsigned short *out) {
	if (op != NULL) {
		unsigned short variable;
		memcpy(&variable, op->arguments + pos, sizeof(unsigned short));
		*out = ntohs(variable);
		return sizeof(unsigned short);
	}
}

static int show_read_int(struct show_operation *op, int pos, int *out) {
	if (op != NULL) {
		int variable;
		memcpy(&variable, op->arguments + pos, sizeof(int));
		*out = ntohl(variable);
		return sizeof(int);
	}
}

static int show_read_float(struct show_operation *op, int pos, float *out) {
	if (op != NULL) {
		float variable;
		memcpy(&variable, op->arguments + pos, sizeof(float));
		*out = variable;
		return sizeof(float);
	}
}


inline static int enough_arguments(const char *function_name, struct show_operation *op, int size) {
	if (op->argument_len < size) {
		fprintf(stderr, "%s: Too small packet\n", function_name);
		operation_remove(op);
		return 0;
	}
	return 1;
}

void show_effect_start(struct show_operation *op) {
	unsigned short effect_id;
	show_read_short(op, 0, &effect_id);

	current_effect_id = effect_id;
}

void show_bartrig_start(struct show_operation *op) {
	int len;
	unsigned short starttime;
	unsigned short endtime;
	len =  show_read_short(op, len, &starttime);
	len =+ show_read_short(op, len, &endtime);

	current_bartrig_id = bartrigs_count;
	
	bartrigs[bartrigs_count].timestamp_start = starttime;
	bartrigs[bartrigs_count].timestamp_end = endtime;
	bartrigs[bartrigs_count].bartrig_id = bartrigs_count++;
}

void show_timestamp_start(struct show_operation *op) {
	int len;
	unsigned short stime;
	
	show_read_short(op, 0, &stime);
	
	current_timestamp_id = timestamps_count;
	
	timestamps[timestamps_count].timestamp = stime;
	timestamps[timestamps_count].timestamp_id = timestamps_count++;
}

void operation_add(struct show_operation *op) {
	int i;
	int opid = -1;

	for (i=0; i < MAX_OPERATIONS; ++i) {
		if (operations[i].allocated == 0) {
			opid = i;
			break;
		}
	}
	if (opid == -1) {
		fprintf(stderr, "%s: Operations storage exceeded\n",__func__);
		return;
	}
	
	memcpy(&operations[opid], op, sizeof(struct show_operation));
	op = &operations[opid];
	
	if (current_effect_id > -1) {
		op->operation_type = OP_EFFECT;
		op->effect_id = current_effect_id;
	} else
	if (current_bartrig_id > -1) {
		op->operation_type = OP_BARTRIG;
		op->bartrig_id = current_bartrig_id;
	} else
	if (current_timestamp_id > -1) {
		op->operation_type = OP_TS;
		op->timestamp_id = current_timestamp_id;
	}
	
}

static void handle_operation(struct show_operation *op) {
	int len = 0;

	switch (op->command) {
		case SHOW_FUNC_EFFECT_START:
			show_effect_start(op);
			break;

		case SHOW_FUNC_EFFECT_END:
			current_effect_id = -1;
			break;

		case SHOW_FUNC_BARTRIG_START:
			show_bartrig_start(op);
			break;

		case SHOW_FUNC_BARTRIG_END:
			current_bartrig_id = -1;

		case SHOW_FUNC_TS_START:
			show_timestamp_start(op);
			break;

		case SHOW_FUNC_TS_END:
			current_timestamp_id = -1;
			break;

		case SHOW_FUNC_FADE:
			operation_add(op);
	}
	
}

static struct show_operation *parse_packet(unsigned char *data, int length) {
	static struct show_operation operation;
	if (length < 1)
		return NULL;

	if (length - 1 > MAX_ARG_LEN)
		return NULL;

	memset(&operation, 0, sizeof(struct show_operation));
	operation.command = data[0];
	memcpy(operation.arguments, data + 1, length - 1);
	operation.argument_len = length - 1;

	if (verbose)
	printf("Parsed command: %d with %d bytes of arguments: arg1:%d\n", operation.command, operation.argument_len, operation.arguments[0]);

	return &operation;
}

static void show_read_datafile() {
	FILE *fp;
	unsigned short packetsize;
	unsigned char magic[] = "showdaemon";
	unsigned char checkmagic[11];
	
	fp = fopen("datafile","r");
	if (fp == NULL) {
		fprintf(stderr, "%s: Cannot open file 'datafile': %s\n", __func__, strerror(errno));
		return;
	}

	fread(&checkmagic, sizeof(checkmagic), 1, fp);
	
	if (memcmp(&magic, &checkmagic, sizeof(magic)) != 0) {
		fprintf(stderr, "%s: Invalid datafile 'datafile'.\n", __func__);
		fclose(fp);
		return;
	}

	while (!feof(fp)) {
		struct show_operation *operation;
		unsigned char buffer[1024];
		int len;
		
		len = fread(&packetsize, sizeof(unsigned short), 1, fp);
		if (ferror(fp) || len != 1) {
			fclose(fp);
			return;
		}
		packetsize = ntohs(packetsize);
		if (packetsize > 1024)
			packetsize = 1024;

		len = fread(&buffer, packetsize, 1, fp);
		if (ferror(fp) || len != 1) {
			fprintf(stderr, "%s: unexpected end of file\n", __func__);
			fclose(fp);
			return;
		}
		operation = parse_packet(buffer, packetsize);
		if (operation == NULL) {
			return;
		}

		handle_operation(operation);
	}
}



static void *functions[] = {
/*
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
*/
};

static void (*find_function(unsigned char func)) (struct show_operation *op, float runtime) {
	int i;
	
	for (i = 0; i < sizeof(functions); i += 2) {
		if ((int)functions[i] == func)
			return functions[i+1];
	}
	return NULL;
}

int main(int argc, char **argv) {
	struct sockaddr_in si_me, si_other;
	int len, slen = sizeof(si_other);
	fd_set selectlist;
	int highsock;
	int optc;
	unsigned char buffer[1024];
	struct timeval updated;
	jack_status_t status;

	struct timeval timeout;

	dmxc_init("127.0.0.1",9118);
	
	// parse options 
	while ((optc = getopt (argc, argv, "v")) != EOF) {
		switch (optc) {
			case 'v':
				verbose = 1;
				break;

		}
	}

	if (argc == 0) {
		fprintf(stderr, "Usage: %s -a [-v]\n", argv[0]);
		return 1;
	}

	show_read_datafile();
	return 0;


	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("show socket");
		return 1;
	}

	memset((void *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(SHOW_UDP_PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (const struct sockaddr*)&si_me, sizeof(si_me)) == -1) {
		perror("show udp port");
		close(sock);
		return 1;
	}

	highsock = sock;

        if ((client = jack_client_open (PROGRAM_NAME,  JackNullOption, &status)) == 0) {
                fprintf (stderr, "JACK: Server not running?\n");
                return 1;
        }

	signal(SIGQUIT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);

	jack_on_shutdown (client, jack_shutdown, 0);

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		return 1;
	}

	
	/* save cursor */
	printf("\e7");
	
	/* Mainloop */
	while (1) {
		jack_position_t current;
		jack_nframes_t frame_time;
		jack_transport_state_t transport_state;
		float position;
		static struct timeval lastsend;
		struct timeval now;
		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;

		FD_ZERO(&selectlist);
		FD_SET(sock, &selectlist);

		gettimeofday(&now, NULL);

                transport_state = jack_transport_query (client, &current);
                frame_time = jack_get_current_transport_frame (client);
		if (transport_state == JackTransportRolling) {
			position = (float)frame_time / current.frame_rate;
		}

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

				if (buffer[0] == 1) {
					show_read_datafile();
				} else {
					printf("Invalid packet received\n");
				}
			}
		}

		/* Come here at least every *timeout* uS */
//		handle_operations();
		gettimeofday(&lastsend, NULL);
		
		if (!verbose && (now.tv_sec > updated.tv_sec)) {
			int i,num_operations = 0;
			for (i = 0; i < MAX_OPERATIONS; ++i) {
				if (operations[i].allocated)
					num_operations++;
			}
			printf("\e8\e[2KCurrent operations: %d", num_operations);
			fflush(stdout);
			gettimeofday(&updated, NULL);
		}
	}
}
