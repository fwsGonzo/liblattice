/*
**
** $Id $
**
** The Lattice (lattice) by remorse@paddylee.com; scalable MMO solutions
** Copyright (C) 2010-2013 Ralph Covelli
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
*/

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#ifdef __linux__
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <fcntl.h>
#else
	#include <winsock2.h>
#endif

#include "lattice_config.h"
#include "struct.h"
#include "globals.h"
#include "send.h"

int ncoord_is_equal(n_coord a, n_coord b) {

    if ((a.x == b.x) && (a.y == b.y) && (a.z == b.z))
        return 1;

    return 0;

}

n_coord wcoord_to_ncoord(w_coord wcoord) {

    n_coord ncoord;

    ncoord.x = wcoord.x >> 8;
    ncoord.y = wcoord.y >> 8;
    ncoord.z = wcoord.z >> 8;

    return ncoord;

}


int serv_in_range_of_serv(n_coord center, n_coord side) {

    if (center.x < 0 || center.x > LATTICE_XMAX) return 0;
    if (center.y < 0 || center.y > LATTICE_YMAX) return 0;
    if (center.z < 0 || center.z > LATTICE_ZMAX) return 0;

    if (side.x < 0 || side.x > LATTICE_XMAX) return 0;
    if (side.y < 0 || side.y > LATTICE_YMAX) return 0;
    if (side.z < 0 || side.z > LATTICE_ZMAX) return 0;

    if ((side.x < center.x - reach) ||
        (side.x > center.x + reach)) return 0;
    if ((side.y < center.y - reach) ||
        (side.y > center.y + reach)) return 0;
    if ((side.z < center.z - reach) ||
        (side.z > center.z + reach)) return 0;

    return 1;


}

int serv_can_connect_to_serv(n_coord center, n_coord side) {

    if (ncoord_is_equal(center, side)) return 0;

    return serv_in_range_of_serv(center, side);

}

int serv_can_connect_to_me(n_coord coord) {

    return serv_can_connect_to_serv(lattice_player.centeredon, coord);
}


void init_neighbor_table(void) {

    memset(neighbor_table, 0, sizeof(server_socket *) * 27 ); // 3*3*3

    return;

}

server_socket *find_neighbor(n_coord coord) {

    int a,b,c;

    if (!serv_can_connect_to_me(coord)) return NULL;

    a = coord.x - lattice_player.centeredon.x + reach;
    b = coord.y - lattice_player.centeredon.y + reach;
    c = coord.z - lattice_player.centeredon.z + reach;

    return neighbor_table[a][b][c];

}

int add_neighbor(n_coord coord, server_socket *s) {

    int a,b,c;

    if (!s) return 0;

    if (!serv_can_connect_to_me(coord)) return 0;

    a = coord.x - lattice_player.centeredon.x + reach;
    b = coord.y - lattice_player.centeredon.y + reach;
    c = coord.z - lattice_player.centeredon.z + reach;

    if (neighbor_table[a][b][c]) return 0;

    neighbor_table[a][b][c]=s;

    return 1;

}

int del_neighbor(n_coord coord) {

    int a,b,c;

    if (!serv_can_connect_to_me(coord)) return 0;

    a = coord.x - lattice_player.centeredon.x + reach;
    b = coord.y - lattice_player.centeredon.y + reach;
    c = coord.z - lattice_player.centeredon.z + reach;

    if (!neighbor_table[a][b][c]) return 0;

    neighbor_table[a][b][c]=NULL;

    return 1;

}


server_socket *connect_server(n_coord coord, struct in_addr ip, port_t port) {

    int sockfd;
    struct sockaddr_in  serv_addr;
    //void *sub;

    if (!serv_in_range_of_serv(lattice_player.centeredon, coord)) {
        return NULL;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        return NULL;
    }

    memset(&serv_addr, 0, sizeof serv_addr);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = ip.s_addr;

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof serv_addr) < 0) {
        return NULL;
    }

    #ifdef __linux__
		int val = fcntl(sockfd, F_GETFL, 0);
		fcntl(sockfd, F_SETFL, val | O_NONBLOCK);
	#else
		unsigned long blocking = 0;
		ioctlsocket(sockfd, FIONBIO, &blocking);
	#endif

/*
    if (!add_neighbor(coord,socket_table+sockfd)) {

        close(sockfd);
        return NULL;
    }

    sub = malloc(sizeof(subsock_server));
    if (!sub) {
        close(sockfd);
        return NULL;
    }
*/

    FD_SET(sockfd, &rtest_set);

    if(sockfd > maxfd)
        maxfd = sockfd;

    socket_table[sockfd].socket = sockfd;
    socket_table[sockfd].wlen = 0;
    socket_table[sockfd].rlen = 0;

    socket_table[sockfd].sendq_head = NULL;
    socket_table[sockfd].sendq_tail = NULL;

    socket_table[sockfd].sendq_length = 0;
    socket_table[sockfd].sendq_max = 0;

    socket_table[sockfd].ip.s_addr = ip.s_addr;
    socket_table[sockfd].port = port;

    socket_table[sockfd].type = SOCKET_SERVER;

/*
    socket_table[sockfd].subsocket = sub;
*/
    socket_table[sockfd].coord.x = coord.x;
    socket_table[sockfd].coord.y = coord.y;
    socket_table[sockfd].coord.z = coord.z;

/*
    if (sendto_one(socket_table+sockfd, "SERVER %d %d %d %s %d %d\n", lattice_player.centeredon.x,
                                                                      lattice_player.centeredon.y,
                                                                      lattice_player.centeredon.z,
                                                                  inet_ntoa(gip), gserverport, gclientport)) {
                 close(sockfd);
                 return NULL;
              }
*/

    return socket_table + sockfd;



}


