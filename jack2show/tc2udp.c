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

/* Listen on UDP Port */
int port  = 9119;


/* Internal */

int socket_descriptor;
int iter  = 0;
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

	if (argc <= 1) {
		fprintf(stderr, "Usage: %s <host>\n", argv[0]); 
		return 1;
	}

	hostbyname = gethostbyname(argv[1]);
	if(hostbyname == NULL) {
		perror("SOCKET Gethostbyname failed");
		exit(errno);
	}

	bzero(&address, sizeof(address)); // empty data structure
	address.sin_family = AF_INET;
	memcpy(&address.sin_addr.s_addr, hostbyname->h_addr, sizeof(address.sin_addr.s_addr));
	address.sin_port = htons(port);
	socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

	if (socket_descriptor == -1) {
		perror("SOCKET: Call failed");
		exit(errno);
	}

	jack_status_t status;

	if ((client = jack_client_open ("jack2tc",  JackNullOption, &status)) == 0) {
		fprintf (stderr, "JACK: Server not running?\n");
		return 1;
	}

	printf("JACK: Connected\n");

	signal (SIGQUIT, signal_handler);
	signal (SIGTERM, signal_handler);
	signal (SIGHUP, signal_handler);
	signal (SIGINT, signal_handler);

	jack_on_shutdown (client, jack_shutdown, 0);

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		return 1;
	}

	printf("JACK: Client active\n");

	unsigned int loops = 0;
	unsigned int termbytes = 0;
	unsigned int udp_problems = 0;

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
					udp_problems++;
				}
			}
		}
		
		if (loops++ > 5) {
			if (termbytes++ > 20) {
				printf("\e[1000;0H");
				termbytes = 0;
			} else {
				if (termbytes > 0) printf("\e[1000;%dH", termbytes+1);
			}

			if (udp_problems == 0) {
				if (transport_state == JackTransportRolling) {
					printf("> ");
				} 
				else {
					printf("# ");
				}
			} else {
				printf("!!");
			}
			
			fflush(stdout);
			// Clear line printf("\e[K");
			loops = 0;
		}

	}

	jack_client_close (client);
	printf("JACK: Client closed\n");
	exit (0);
}
