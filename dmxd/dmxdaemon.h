#ifndef _DMXDAEMON_H
#define _DMXDAEMON_H 1
#include <sys/socket.h>
#include <time.h>

#define DMXD_UDP_PORT 9118

#define DMXD_MAX_ARG_LEN 128

enum dmxd_operation_flags {
	DMXD_OP_NONE = 0,
	DMXD_OP_OVERRIDE = 1
};

struct dmxd_operation {
	char active;
	char allocated;
	struct timeval start_time;
	unsigned short uniqueid;
	unsigned short channel;
	unsigned char command;
	enum dmxd_operation_flags flags;
	int operation_id;
	int transaction_id;
	int connection_id;
	char arguments[DMXD_MAX_ARG_LEN];
	int argument_len;
};

struct dmxd_connection {
	char inuse;
	struct sockaddr_in *from;
	int transaction_id;
};

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
	#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

#endif