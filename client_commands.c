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
#include "globals.h"


int c_p(w_coord wcoord, b_coord bcoord) {

    return (sendto_one(neighbor_table[1][1][1], "P %d %d %d %d %d %d\n", wcoord.x, wcoord.y, wcoord.z, bcoord.x, bcoord.y, bcoord.z));

}
