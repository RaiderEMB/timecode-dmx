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


#define FUNC_EFFECT_START 1
#define FUNC_EFFECT_END 2
#define FUNC_BARTRIG_START 3
#define FUNC_BARTRIG_END 4
#define FUNC_TS_START 5
#define FUNC_TS_END 6
#define FUNC_RESET 7
#define FUNC_FADE 10
#define FUNC_BLINK 11
#define FUNC_LOCK 12
#define FUNC_CONTROL 13

#define PROGRAM_NAME "showdaemon"

struct show_operation {
	char allocated;

	enum operation_type {
		OP_EFFECT,
		OP_BARTRIG,
		OP_TS,
		OP_COMMAND,
	} operation_type;

	unsigned char arguments[MAX_ARG_LEN];
	int argument_len;

	unsigned char command;

	int connection_id;
	int effect_id;
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

struct show_connection {
	char inuse;
	struct sockaddr_in *from;
	int transaction_id;
};

static struct show_connection connections[MAX_CONNECTIONS];
static struct show_operation  operations[MAX_OPERATIONS];

void signal_handler (int sig) {
        jack_client_close (client);

        fprintf (stderr, "signal received, exiting ...\n");
        exit (0);
}

void jack_shutdown (void *arg) {
        exit (1);
}

static struct show_operation *parse_packet(unsigned char *data, int length) {
	static struct show_operation operation;
	if (length < 4)
		return NULL;

	if (length - 4 > MAX_ARG_LEN)
		return NULL;

	memset(&operation, 0, sizeof(struct show_operation));
	operation.command = data[0];
	memcpy(operation.arguments, data + 4, length - 4);
	operation.argument_len = length - 4;

	if (verbose)
	printf("Parsed command: %d\n", operation.command);

	return &operation;
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
	dmxc_send(DMXD_FUNC_FADE, 123, 0, 255, 10.f);
	dmxc_send(DMXD_FUNC_BLINK, 124, 0, 255, 0.5f, 0.5f, 10);

	return 0;
	
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
				int connection_id = find_connection(&si_other);
				if (connection_id == -1) {
					if (connection_id = add_connection(&si_other) == -1) {
						/* No more connections available, ignore packet */
						continue;
					}
				}
				struct show_operation* operation = parse_packet(buffer, len);
				if (operation != NULL) {
					operation->connection_id = connection_id;

					if (verbose)
						printf("Connection_id = %d\n", operation->connection_id);

//					add_operation(operation);

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
