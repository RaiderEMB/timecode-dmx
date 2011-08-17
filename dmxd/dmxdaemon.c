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
#include <time.h>
#include "dmxdaemon.h"

static int sock;

static int operation_id = 1;
static int transaction_id = 1;

#define MAX_CONNECTIONS 20
#define MAX_OPERATIONS 1024
#define BUFLEN 1024

static struct dmxd_connection connections[MAX_CONNECTIONS];
static struct dmxd_operation operations[MAX_OPERATIONS];

#define FUNC_FADE 1
#define FUNC_TRANSACTION_START 14
#define FUNC_TRANSACTION_END 15

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
static int dmxd_read_float(struct dmxd_operation *op, int pos, float *out) {
	if (op != NULL) {
		float variable;
		memcpy(&variable, op->arguments + pos, sizeof(float));
		*out = ntohl(variable);
		return sizeof(float);
	}
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

	printf("Parsed command: %d, uniqueid: %d\n", operation.command, operation.uniqueid);
	
	return &operation;
}

static void operation_remove(struct dmxd_operation *op) {
	printf("Removing oid: %d\n", op->operation_id);
	if (op != NULL && op->operation_id >= 0) {
		operations[op->operation_id].active = 0;
		operations[op->operation_id].allocated = 0;
	}
}

static void operation_activate(struct dmxd_operation *op) {
	printf("Activating oid: %d, (%d)\n", op->operation_id, op->command);
	if (op != NULL) {
		operations[op->operation_id].active = 1;
		gettimeofday(&operations[op->operation_id].start_time, NULL);
	}
}

static void f_fade(struct dmxd_operation *op, float runtime) {
	int len = 0;
	short channel;
	unsigned char fromval;
	unsigned char toval;
	float timespan;
	
	if (op->argument_len < 8) {
		fprintf(stderr, "f_fade: Too small packet\n");
		operation_remove(op);
		return;
	}
	
	len += dmxd_read_short(op, len, &channel);
	len += dmxd_read_byte(op, len, &fromval);
	len += dmxd_read_byte(op, len, &toval);
	len += dmxd_read_float(op, len, &timespan);
	
//	printf("Channel: %d\nFromval: %d\nToval: %d\nTimespan: %f\n\n", channel, fromval, toval, timespan);
	printf("%d: %02d %f\n", op->operation_id, fromval + (int)((float)((toval-fromval) * (float)(runtime/timespan))), runtime);
	//printf("Ran fade function With channel %d for %f of %f seconds\n", channel, runtime, timespan);
	if (runtime >= timespan) {
		operation_remove(op);
	}
}

static void f_transaction_end(struct dmxd_operation *op, float runtime) {
	printf("Ran transaction end function\n");
	if (connections[op->connection_id].transaction_id != -1) {
		int i;
		for (i = 0; i < MAX_OPERATIONS; ++i) {
			if (operations[i].allocated && 
				operations[i].active == 0 && 
				operations[i].transaction_id == op->transaction_id) {
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
	
	printf("Ran transaction start function\n");
	printf("Assigned transaction id %d\n", transaction_id);
	connections[op->connection_id].transaction_id = transaction_id;

	memcpy(buffer, &n_uniqueid, sizeof(n_uniqueid));
	buffer[2] = op->command;
	memcpy(buffer+3, &n_transaction_id, sizeof(n_transaction_id));

	bufferlen = sizeof(n_uniqueid) + 1 + sizeof(n_transaction_id);

	sendto(sock, buffer, bufferlen, MSG_DONTWAIT, (const struct sockaddr*)connections[op->connection_id].from, sizeof(connections[op->connection_id].from));
	transaction_id++;

	printf("Sent packet back about transaction_id=%d\n", transaction_id);

	operation_remove(op);
}

static void *functions[] = {
	(void *)1, (void *)f_fade,
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
			printf("Connection added.\n");
			return i;
		}
	}
	fprintf(stderr, "Tried to accept new connection, but no more connectionslots available\n");
	return -1;
}

static int add_operation(struct dmxd_operation *operation) {
	int i;
	
	for (i = 0; i < MAX_OPERATIONS; ++i) {
		if (operations[i].allocated == 0) {
			operation->allocated = 1;
			operation->operation_id = i;
			memcpy(&operations[i], operation, sizeof(struct dmxd_operation));
			printf("Operation added with command: %d, Active: %d, Operation-id: %d\n", operation->command, operations[i].active, i);
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
			printf("Handling operation %d (%d)\n", operations[i].operation_id, operations[i].command);
			func = find_function(operations[i].command);
			if (func != NULL) {
				float duration;
				struct timeval now;
				gettimeofday(&now, NULL);
				duration = (now.tv_sec - operations[i].start_time.tv_sec) + ((now.tv_usec / 1000000.f) - (operations[i].start_time.tv_usec / 1000000.f));
				func(&operations[i], duration);
			}
		}
	}
	if (todo > 0)
	  printf("Handeled %d jobs\n", todo);
}

int main(int argc, char **argv) {
	struct sockaddr_in si_me, si_other;
	int len, slen = sizeof(si_other);
	unsigned char buffer[BUFLEN];
	fd_set selectlist;
	int highsock;
	struct timeval timeout;
	
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		return 1;
	}

	memset((void *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(DMXD_UDP_PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(sock, (const struct sockaddr*)&si_me, sizeof(si_me)) == -1) {
		perror("bind");
		close(sock);
		return 1;
	}
	
	highsock = sock;
	
	/* Mainloop */
	while (1) {
		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;
		
		FD_ZERO(&selectlist);
		FD_SET(sock, &selectlist);
		
		int ready = select(highsock + 1, &selectlist, 0, 0, &timeout);
		
		if (ready > 0) {
			if (FD_ISSET(sock, &selectlist)) {
				len = recvfrom(sock, buffer, BUFLEN, 0, (struct sockaddr *)&si_other, &slen);
				if (len == -1) {
					perror("recvfrom");
					return 1;
				}
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

					printf("Connection_id = %d, transaction_id = %d\n", operation->connection_id, operation->transaction_id);

					add_operation(operation);

					/* No transaction, run immediately */
					if (operation->transaction_id == -1) {
						operation_activate(operation);
					} else
					/* Transaction start/end commands has to be run immediately anyways */
					if (operation->command == FUNC_TRANSACTION_START || operation->command == FUNC_TRANSACTION_END) {
						operation_activate(operation);
					}
				} else {
					printf("Invalid packet received\n");
				}
			}
		}
		
		/* Come here at least every *timeout* uS */
		handle_operations();
	}
}
