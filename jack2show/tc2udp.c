#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include <getopt.h>
#include <jack/jack.h>
#include <jack/transport.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 


int port = 9119;

int socket_descriptor;
int iter = 0;
int errno = 0;
ssize_t sendto_rc;
int close_rc;
char buf[80];
struct sockaddr_in address;
struct hostent *hostbyname;


jack_client_t *client;


void jack_shutdown (void *arg) {
	exit (1);
}

void signal_handler (int sig) {
	jack_client_close (client);

	//Most people don''t bother to check the return code returned by the close function
	close_rc = close (socket_descriptor);
	if (close_rc == -1)
	{
		perror("close call failed");
		exit(errno);
	}


	fprintf (stderr, "signal received, exiting ...\n");
	exit (0);
}

int main (int argc, char *argv[]) {
	hostbyname = gethostbyname("172.16.1.6");
	if(hostbyname == NULL) {
		perror("Gethostbyname failed");
		exit(errno);
	}

	bzero(&address, sizeof(address)); // empty data structure
	address.sin_family = AF_INET;
	memcpy(&address.sin_addr.s_addr, hostbyname->h_addr, sizeof(address.sin_addr.s_addr));
	address.sin_port = htons(port);
	socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

	if (socket_descriptor == -1) {
		perror("socket call failed");
		exit(errno);
	}


	if ((client = jack_client_new ("jack2tc")) == 0) {
		fprintf (stderr, "jack server not running?\n");
		return 1;
	}

	printf("Jack: Tilkoblet\n");

	signal (SIGQUIT, signal_handler);
	signal (SIGTERM, signal_handler);
	signal (SIGHUP, signal_handler);
	signal (SIGINT, signal_handler);

	jack_on_shutdown (client, jack_shutdown, 0);

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		return 1;
	}

	while (1) {

		usleep(20000);

		jack_position_t current;
		jack_transport_state_t transport_state;
		jack_nframes_t frame_time;
		jack_time_t jacktime;

		transport_state = jack_transport_query (client, &current);
		frame_time = jack_get_current_transport_frame (client);
		jacktime = jack_frames_to_time(client, frame_time);
		
		if (transport_state == JackTransportRolling) {
			if (current.valid & JackPositionBBT) {

				sprintf (buf, "%d:%d:%f\n", 
						current.bar, 
						current.beat, 
						(float)frame_time / current.frame_rate
				);
				//printf("%s", buf);
	
				sendto_rc = sendto(socket_descriptor, buf, sizeof(buf), 0, (struct sockaddr *)&address, sizeof(address));

				if (sendto_rc == -1) {
					perror("sendto: failed, moving on");
				}
			}
		}
	}

	jack_client_close (client);
	exit (0);
}
