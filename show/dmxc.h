#ifndef _DMXC_H
#define _DMXC_H

enum dmxd_commands {
	FUNC_FADE = 1,
	FUNC_BLINK,
	FUNC_CONTROL,
	FUNC_LOCK,
	FUNC_MAX = 6,
	FUNC_MIN,
	FUNC_ADD,
	FUNC_SUB,
	FUNC_CANCEL,
	FUNC_CANCELTRANS,
	FUNC_SCALEMAX,
	FUNC_SCALEMIN,
	FUNC_TRANSACTION_START,
	FUNC_TRANSACTION_END
};

int dmxc_init(char *hostname, unsigned short port);
int dmxc_send(enum dmxd_commands command, ...);
int dmxc_sendf(enum dmxd_commands command, char flag, ...);

#endif
