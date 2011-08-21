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
	FUNC_TRANSACTION_START,
	FUNC_TRANSACTION_END
};

int dmxc_init(char *hostname, unsigned short port);
int dmxc_send(enum dmxd_commands command, ...);
int dmxc_sendf(enum dmxd_commands command, char flag, ...);

#endif
