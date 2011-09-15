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
#define MAX_OPERATIONS (512*10)
#define MAX_LOCAL_OPERATIONS 512
#define MAX_EFFECTS 1024
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

	unsigned char step;
	float length;

	int connection_id;
	unsigned short effect_id;
	int bartrig_id;
	int timestamp_id;
};

struct show_timestamp {
	int timestamp_id;
	int timestamp;
	int operation_count;

	struct show_operation *operations[MAX_LOCAL_OPERATIONS];
};

struct show_bartrig {
	int bartrig_id;
	int timestamp_start;
	int timestamp_end;
};

struct show_effect {
	int effect_id;
	char step_count;
	int operation_count;
	int length;

	float step_lengths[256];
	struct show_operation operations[MAX_LOCAL_OPERATIONS];
};

struct show_run_effect {
	unsigned short offset;
	unsigned short effect_id;
	int timestamp_start;
	int active;
	int stepsrun;
};

static int current_effect_id = -1;
static int current_bartrig_id = -1;
static int current_timestamp_id = -1;

static int bartrigs_count = 0;
static int timestamps_count = 0;

static struct show_operation  operations[MAX_OPERATIONS];
static struct show_timestamp  timestamps[MAX_OPERATIONS];
static struct show_bartrig    bartrigs[MAX_OPERATIONS];

static int effectid_count = 0;
static struct show_effect effects[MAX_EFFECTS];

static struct show_run_effect run_effects[MAX_EFFECTS];
static int show_run_effect_count = 0;

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


char *timestamp_display(int timestamp) {
	static char string[12];
	int ms = timestamp % 1000;
	timestamp = timestamp / 1000;

	sprintf(string, "%02d:%02d:%02d.%03d",
		(int)(timestamp/3600.f),
		(int)(timestamp/60.f) % 60,
		timestamp % 60,
		ms
	);
	return string;
}

void show_effect_start(struct show_operation *op) {
	int i, existing = 0;
	unsigned short effect_id;
	show_read_short(op, 0, &effect_id);

	current_effect_id = effect_id;

	for (i = 0; i < MAX_OPERATIONS; ++i) {
		if (operations[i].allocated && operations[i].effect_id == current_effect_id) {
			existing = 1;
			break;
		}
	}

	if (!existing) {
		effectid_count++;
	}
}

void show_bartrig_start(struct show_operation *op) {
	int len = 0;
	unsigned int starttime;
	unsigned int endtime;
	len =  show_read_int(op, len, &starttime);
	len =+ show_read_int(op, len, &endtime);

	current_bartrig_id = bartrigs_count;
	
	bartrigs[bartrigs_count].timestamp_start = starttime;
	bartrigs[bartrigs_count].timestamp_end = endtime;
	bartrigs[bartrigs_count].bartrig_id = bartrigs_count++;
}

void show_timestamp_start(struct show_operation *op) {
	unsigned int stime;
	
	show_read_int(op, 0, &stime);
	
	current_timestamp_id = timestamps_count;
	
	timestamps[timestamps_count].timestamp = stime;
	timestamps[timestamps_count].timestamp_id = timestamps_count++;
	timestamps[timestamps_count].operation_count = 0;
}

void operation_add(struct show_operation *op, float length) {
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

	op->allocated = 1;

	op->length = length;

	show_read_byte(op, 0, &(op->step));

	printf("Adding ");

	if (current_effect_id > -1) {
		op->operation_type = OP_EFFECT;
		op->effect_id = current_effect_id;
		printf("effect(%d)", current_effect_id);
	} else
	if (current_bartrig_id > -1) {
		op->operation_type = OP_BARTRIG;
		op->bartrig_id = current_bartrig_id;
		printf("bar-trigger(%d) (", current_bartrig_id);
		printf("%s to ", timestamp_display(bartrigs[current_bartrig_id].timestamp_start));
		printf("%s)", timestamp_display(bartrigs[current_bartrig_id].timestamp_end));
	} else
	if (current_timestamp_id > -1) {
		op->operation_type = OP_TS;
		op->timestamp_id = current_timestamp_id;
		printf("timestamped(%d)", current_timestamp_id);
		timestamps[current_timestamp_id].operations[timestamps[current_timestamp_id].operation_count++] = op;
	}

	printf(" command id %d, with step %d, time to run: %f\n", op->command, op->step, op->length);
	printf(" debug: Args len: %d, first arg: %d\n", op->argument_len, op->arguments[0]);
}

static void show_effect_add(struct show_operation *op) {
	int i;
	printf("Spawn-effect added.\n");
	for (i = 0; i < MAX_EFFECTS; ++i) {
		if (run_effects[i].active == 0) {
			run_effects[i].active = 1;
			show_read_short(op, 0, &(run_effects[i].effect_id));
			show_read_short(op, 2, &(run_effects[i].offset));
			run_effects[i].timestamp_start = timestamps[current_timestamp_id].timestamp;
			printf("Spawn effect id: %d at %s\n", i, timestamp_display(run_effects[i].timestamp_start));
			show_run_effect_count++;
			return;
		}
	}
}

static void handle_operation(struct show_operation *op) {
	float length1 = 0;
	float length2 = 0;
	int times = 1;

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
			break;

		case SHOW_FUNC_TS_START:
			show_timestamp_start(op);
			break;

		case SHOW_FUNC_TS_END:
			current_timestamp_id = -1;
			break;

		case SHOW_FUNC_EFFECT:
			if (current_timestamp_id > 0)
				show_effect_add(op);
			else
				fprintf(stderr, "Could not add effect without a timestamp scope\n");
			break;

		case SHOW_FUNC_BLINK:
			show_read_float(op,6,&length1);
			show_read_int(op,10,&times);
		case SHOW_FUNC_FADE:
			show_read_float(op,5,&length2);
			operation_add(op, (length1 + length2) * times);
			break;

		case SHOW_FUNC_LOCK:
			show_read_float(op, 4, &length1);
			operation_add(op, length1);
			break;
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

		printf("Reading %d bytes\n", packetsize);
		len = fread(&buffer, packetsize, 1, fp);
		if (ferror(fp) || len != 1) {
			fprintf(stderr, "%s: unexpected end of file at position %ld while reading %d bytes\n", __func__, ftell(fp), packetsize);
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

void send_command(struct show_operation *op) {
	struct dmxc_packet packet;
	int i;

	dmxc_init("127.0.0.1",9118);

	switch (op->command)  {
		case SHOW_FUNC_LOCK:
			dmxc_packet_init(&packet, DMXD_FUNC_LOCK, 1);
			break;

		case SHOW_FUNC_BLINK:
			dmxc_packet_init(&packet, DMXD_FUNC_BLINK, 1);
			break;

		case SHOW_FUNC_FADE:
			dmxc_packet_init(&packet, DMXD_FUNC_FADE, 1);
			break;

		case SHOW_FUNC_CONTROL:
			dmxc_packet_init(&packet, DMXD_FUNC_CONTROL, 1);
			break;

		default:
			return;
	}

	for (i = 0; i < op->argument_len; ++i) {
		dmxc_packet_add_char(&packet, op->arguments[i]);
	}
	//printf("Sending command %d, len %d\n", packet.data[0], packet.length);
	dmxc_udp_send(&packet);
}

int main(int argc, char **argv) {
	struct sockaddr_in si_me, si_other;
	int i, ii, len, slen = sizeof(si_other);
	fd_set selectlist;
	int highsock;
	int optc;
	unsigned char buffer[1024];
	struct timeval updated;
	jack_status_t status;

	struct timeval timeout;

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
	
	printf("Raw data loaded\n");
	for (i = 0; i < MAX_OPERATIONS; ++i) {
		if (operations[i].allocated == 1) {
			if (operations[i].effect_id > 0) {
				printf("\tEffect: %02d Step %02d: command: %d length: %f\n", operations[i].effect_id, operations[i].step, operations[i].command, operations[i].length);
			}
		}
	}

	for (i = 1; i < effectid_count + 1; ++i) {
		effects[i].effect_id = i;
		int operationi;
		for (ii = 0; ii < MAX_OPERATIONS; ++ii) {
			if (operations[ii].allocated && operations[ii].effect_id == i) {
				int stepexists=0;
				printf("Found step %d in effect %d\n", operations[ii].step, i);

				// Count steps
				for (operationi = 0; operationi < effects[i].operation_count; ++operationi) {
					if (effects[i].operations[operationi].step > 0 && effects[i].operations[operationi].step == operations[ii].step)
						stepexists=1;
				}
				if (!stepexists)
					effects[i].step_count++;

				effects[i].operations[effects[i].operation_count++] = operations[ii];
			}
		}
	}
	
	/* DO NOT REMOVE */
	printf("Organized data: (effects: %d)\n", effectid_count);
	for (i = 1; i < effectid_count + 1; ++i) {
		int stepi = 0;
		int operationi = 0;
		float totallen = 0;
		printf("\tEffect %d, operations: %d, steps: %d\n", i, effects[i].operation_count, effects[i].step_count);
		
		// Find total length of each step
		for (stepi = 1; stepi < effects[i].step_count+1; ++stepi) {
			for (operationi = 0; operationi < effects[i].operation_count; ++operationi) {
				if (effects[i].operations[operationi].step == stepi && effects[i].operations[operationi].length > effects[i].step_lengths[stepi]) {
					effects[i].step_lengths[stepi] = effects[i].operations[operationi].length;
				}
			}
		}
		printf("\t\tOperations:\n");
		for (stepi = 0; stepi < effects[i].operation_count ; ++stepi) {
			printf("\t\t\tStep %d, length %f\n", effects[i].operations[stepi].step, effects[i].operations[stepi].length);
		}

		printf("\t\tStep lengths:\n");
		for (stepi = 1; stepi < effects[i].step_count+1; ++stepi) {
			printf("\t\t\tStep %02d: lengths: %f\n", stepi, effects[i].step_lengths[stepi]);
			totallen += effects[i].step_lengths[stepi];
		}
		effects[i].length = totallen;
	}

	printf("Timed events: (count: %d)\n", timestamps_count);

	for (i = 0; i < timestamps_count; ++i) {
		int oi=0;
		printf("\t(%s): (id=%d, count=%d)\n", timestamp_display(timestamps[i].timestamp), i, timestamps[i].operation_count);
		for (oi = 0; oi < timestamps[i].operation_count; ++oi) {
			unsigned short channel;
			show_read_short(timestamps[i].operations[oi], 0, &channel);
			printf("\t\tfunc_%d Channel: %d\n", timestamps[i].operations[oi]->command, channel);
		}
	}

//	return 0;


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

	printf("Waiting for jack timecode\n");
	
	/* save cursor */
	printf("\e7");
	
	/* Mainloop */
	while (1) {
		static int last_i = 0;
		static int last_timestamp = 0;
		jack_position_t current;
		jack_nframes_t frame_time;
		jack_transport_state_t transport_state;
		float position;
		static struct timeval lastsend;
		struct timeval now;
		timeout.tv_sec = 0;
		timeout.tv_usec = 4000;

		FD_ZERO(&selectlist);
		FD_SET(sock, &selectlist);

		gettimeofday(&now, NULL);

                transport_state = jack_transport_query (client, &current);
                frame_time = jack_get_current_transport_frame (client);
		if (transport_state == JackTransportRolling) {
			position = (float)frame_time / current.frame_rate;

			/* Have we traveled back in time? */
			if (last_timestamp == 0 || position * 1000 < last_timestamp) {
				last_timestamp = position * 1000;
				last_i = 0;

				/* invalidate all pending effects */
				//for (i=0; i < MAX_EFFECTS; ++i) {
				//	run_effects[i].active = 0;
				//}
			}

			int effect_count = 0;
			for (i = 0; i < MAX_EFFECTS; ++i) {
				if (run_effects[i].active == 1) {
					int start_time = run_effects[i].timestamp_start;
					if (last_timestamp >= start_time) {
						struct show_effect *eff = &(effects[run_effects[i].effect_id]);
						int o;
						for (o = 0; o < eff->step_count; ++o)  {
							if (start_time +  (eff->step_lengths[o] * 1000) > position * 1000) {
								break;
							}
						}
						
						int current_step = o;
						int has_done_step = 0;
						if (run_effects[i].stepsrun >= current_step)
							break;

						printf("Maybe firing effect job id %d, effect id %d, step %d\n", i, eff->effect_id, o);

						for (o = 0; o < eff->operation_count; ++o) {
							if (eff->operations[o].step == current_step) {
								printf("%s: FIRE EFFECT %d STEP %d\n", timestamp_display(position * 1000), eff->operations[o].command, current_step);
								//TODO: Channel + offset
								send_command(&eff->operations[o]);
								has_done_step = 1;
							}
						}
						if (has_done_step && current_step > run_effects[i].stepsrun)
							run_effects[i].stepsrun = current_step;
					}
					effect_count++;
				}
				if (effect_count == show_run_effect_count)
					break;
			}
			for (i = last_i; i < timestamps_count; ++i) {
				int oi=0;
				if (timestamps[i].timestamp > last_timestamp && timestamps[i].timestamp <= position * 1000) {
					printf("Jack pos: %f\n", position);
					for (oi = 0; oi < timestamps[i].operation_count; ++oi) {
						unsigned short channel;
						show_read_short(timestamps[i].operations[oi], 0, &channel);
						printf("\t\tSpawn func_%d Channel: %d\n", timestamps[i].operations[oi]->command, channel);
						send_command(timestamps[i].operations[oi]);
					}
					last_i = i + 1;
				}
			}
			last_timestamp = position * 1000;
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
		
		if (now.tv_sec > updated.tv_sec) {
			if (transport_state == JackTransportRolling) {
				struct show_operation op;
				op.command = SHOW_FUNC_CONTROL;
				op.argument_len = 0;
				send_command(&op);
			}
			int i,num_operations = 0;
			for (i = 0; i < MAX_OPERATIONS; ++i) {
				if (operations[i].allocated)
					num_operations++;
			}
			//printf("\e8\e[2KCurrent operations: %d", num_operations);
			fflush(stdout);
			gettimeofday(&updated, NULL);
		}
	}
}
