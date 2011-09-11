#ifndef _DMXC_H
#define _DMXC_H

enum dmxd_commands {
	DMXD_FUNC_FADE = 1,
	DMXD_FUNC_BLINK,
	DMXD_FUNC_CONTROL,
	DMXD_FUNC_LOCK,
	DMXD_FUNC_MAX = 6,
	DMXD_FUNC_MIN,
	DMXD_FUNC_ADD,
	DMXD_FUNC_SUB,
	DMXD_FUNC_CANCEL,
	DMXD_FUNC_CANCELTRANS,
	DMXD_FUNC_SCALEMAX,
	DMXD_FUNC_SCALEMIN,
	DMXD_FUNC_TRANSACTION_START,
	DMXD_FUNC_TRANSACTION_END
};

/* Easy api */
int dmxc_init(char *hostname, unsigned short port);
int dmxc_send(enum dmxd_commands command, ...);
int dmxc_sendf(enum dmxd_commands command, char flag, ...);


/* Advanced api */
struct dmxc_packet {
	unsigned short uniqueid;
	int length;
	char data[1024];
};

int dmxc_packet_init(struct dmxc_packet *packet, enum dmxd_commands command, char flag);
int dmxc_packet_add_char(struct dmxc_packet *packet, unsigned char data);
int dmxc_packet_add_short(struct dmxc_packet *packet, unsigned short data);
int dmxc_packet_add_int(struct dmxc_packet *packet, unsigned int data);
int dmxc_packet_add_float(struct dmxc_packet *packet, float data);
int dmxc_udp_send(struct dmxc_packet *packet);

#endif
