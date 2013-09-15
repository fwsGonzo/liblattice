#include <stdlib.h>

#ifdef __linux__
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#else
	#include <winsock2.h>
#endif

#include "lattice_config.h"
#include "serversocket.h"
#include "struct.h"

server_socket *socket_table = NULL;

server_socket *neighbor_table[3][3][3];

fd_set rready_set;
fd_set wready_set;
fd_set rtest_set;
fd_set wtest_set;

int input_sock=-1;

int maxfd=0;

struct timeval now;

int reach=1; // dont change me

n_coord my_coord = {0, 0, 0};

lattice_user lattice_player;

void (*gcallback)();
