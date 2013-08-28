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
#include "send.h"

int s_ping(struct server_socket *src, int argc, char **argv) {

    if (!src) return 0;

    if (sendto_one(src, "PONG\n")) return 1;

    return 0;

}

