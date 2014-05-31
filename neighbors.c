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
#include <stdio.h>
#include <time.h>

#ifdef __linux__
#include <sys/time.h>
#include <unistd.h>
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <fcntl.h>
#else
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#ifndef __MINGW32__
 #include "forwin.h"
#else
 #include <sys/time.h>
 #include <unistd.h>
#endif
#endif

#include "lattice_config.h"
#include "serversocket.h"
#include "struct.h"
#include "globals.h"
#include "send.h"
#include "sched.h"
#include "macros.h"
#include "lattice_packet.h"

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

// -----------------------

int user_is_within_server_border(w_coord wcoord, n_coord ncoord) {
    w_coord min_wcoord;
    w_coord max_wcoord;

    min_wcoord.x = ncoord.x << 8;
    min_wcoord.y = ncoord.y << 8;
    min_wcoord.z = ncoord.z << 8;
    max_wcoord.x = (ncoord.x << 8) | 0x000000FF;
    max_wcoord.y = (ncoord.y << 8) | 0x000000FF;
    max_wcoord.z = (ncoord.z << 8) | 0x000000FF;

    if (wcoord.x < min_wcoord.x || wcoord.x > max_wcoord.x) return 0;
    if (wcoord.y < min_wcoord.y || wcoord.y > max_wcoord.y) return 0;
    if (wcoord.z < min_wcoord.z || wcoord.z > max_wcoord.z) return 0;

    return 1;

}

int user_can_center_to_me(w_coord wcoord) {

    if (wcoord.x < lattice_player.my_min_wcoord.x || wcoord.x > lattice_player.my_max_wcoord.x) return 0;
    if (wcoord.y < lattice_player.my_min_wcoord.y || wcoord.y > lattice_player.my_max_wcoord.y) return 0;
    if (wcoord.z < lattice_player.my_min_wcoord.z || wcoord.z > lattice_player.my_max_wcoord.z) return 0;

    return 1;

}

int user_is_within_outer_border(w_coord wcoord, n_coord ncoord) {
    w_coord min_wcoord;
    w_coord max_wcoord;

    min_wcoord.x = ncoord.x << 8;
    min_wcoord.y = ncoord.y << 8;
    min_wcoord.z = ncoord.z << 8;
    max_wcoord.x = (ncoord.x << 8) | 0x000000FF;
    max_wcoord.y = (ncoord.y << 8) | 0x000000FF;
    max_wcoord.z = (ncoord.z << 8) | 0x000000FF;

    if (wcoord.x < min_wcoord.x - VISUAL_UNIT || wcoord.x > max_wcoord.x + VISUAL_UNIT) return 0;
    if (wcoord.y < min_wcoord.y - VISUAL_UNIT || wcoord.y > max_wcoord.y + VISUAL_UNIT) return 0;
    if (wcoord.z < min_wcoord.z - VISUAL_UNIT || wcoord.z > max_wcoord.z + VISUAL_UNIT) return 0;

    return 1;

}


int user_is_within_inner_border(w_coord wcoord, n_coord ncoord) {
    w_coord min_wcoord;
    w_coord max_wcoord;

    min_wcoord.x = ncoord.x << 8;
    min_wcoord.y = ncoord.y << 8;
    min_wcoord.z = ncoord.z << 8;
    max_wcoord.x = (ncoord.x << 8) | 0x000000FF;
    max_wcoord.y = (ncoord.y << 8) | 0x000000FF;
    max_wcoord.z = (ncoord.z << 8) | 0x000000FF;

    if (wcoord.x < min_wcoord.x + VISUAL_UNIT || wcoord.x > max_wcoord.x - VISUAL_UNIT) return 0;
    if (wcoord.y < min_wcoord.y + VISUAL_UNIT || wcoord.y > max_wcoord.y - VISUAL_UNIT) return 0;
    if (wcoord.z < min_wcoord.z + VISUAL_UNIT || wcoord.z > max_wcoord.z - VISUAL_UNIT) return 0;

    return 1;

}

int user_can_recenter_to_me(w_coord wcoord, n_coord from_ncoord) {

    if (!serv_can_connect_to_me(from_ncoord)) return 0;

    if (user_is_within_outer_border(wcoord, from_ncoord)) return 0;

    return 1;

}


// -----------------------


void init_neighbor_table(void) {

    memset(neighbor_table, 0, sizeof(server_socket *) * 27 ); // 3*3*3

    return;

}

server_socket *find_neighbor_bysock(server_socket *sock) {

    server_socket *s;
    int x;
    int y;
    int z;

    if (!sock) return NULL;

    for (x=0;x<3;x++) for (y=0;y<3;y++) for (z=0;z<3;z++)
        if ((s=neighbor_table[x][y][z]))
            if (s == sock)
                return s;

    return NULL;

}


server_socket *find_neighbor(n_coord coord) {

    int a,b,c;

    if (!serv_in_range_of_serv(lattice_player.centeredon, coord)) return NULL;

    a = coord.x - lattice_player.centeredon.x + reach;
    b = coord.y - lattice_player.centeredon.y + reach;
    c = coord.z - lattice_player.centeredon.z + reach;

    return neighbor_table[a][b][c];

}


int user_can_walk_on_me(w_coord wcoord) {

    n_coord ncoord;

    if (wcoord.x < lattice_player.my_min_wcoord.x - VISUAL_UNIT || wcoord.x > lattice_player.my_max_wcoord.x + VISUAL_UNIT) return 0;
    if (wcoord.y < lattice_player.my_min_wcoord.y - VISUAL_UNIT || wcoord.y > lattice_player.my_max_wcoord.y + VISUAL_UNIT) return 0;
    if (wcoord.z < lattice_player.my_min_wcoord.z - VISUAL_UNIT || wcoord.z > lattice_player.my_max_wcoord.z + VISUAL_UNIT) return 0;

    ncoord = wcoord_to_ncoord(wcoord);

    if ( !ncoord_is_equal(ncoord, lattice_player.centeredon) && !find_neighbor(ncoord) ) return 0;

    return 1;

}



int add_neighbor(n_coord coord, server_socket *s) {

    int a,b,c;

    if (!s) return 0;

    if (!serv_in_range_of_serv(lattice_player.centeredon, coord)) return 0;

    a = coord.x - lattice_player.centeredon.x + reach;
    b = coord.y - lattice_player.centeredon.y + reach;
    c = coord.z - lattice_player.centeredon.z + reach;

    if (neighbor_table[a][b][c]) return 0;

    neighbor_table[a][b][c]=s;

    return 1;

}

int del_neighbor(n_coord coord) {

    int a,b,c;

    if (!serv_in_range_of_serv(lattice_player.centeredon, coord)) return 0;

    a = coord.x - lattice_player.centeredon.x + reach;
    b = coord.y - lattice_player.centeredon.y + reach;
    c = coord.z - lattice_player.centeredon.z + reach;

    if (!neighbor_table[a][b][c]) return 0;

    neighbor_table[a][b][c]=NULL;

    return 1;

}


server_socket *connect_server(n_coord coord, struct in_addr ip, port_t port, int *error) {

    int sockfd;
    struct sockaddr_in  serv_addr;

    struct timeval tv;

    #ifdef _WIN32
        unsigned long blocking;
    #endif

    if (!serv_in_range_of_serv(lattice_player.centeredon, coord)) {
        if(error) *error = -1;
        return NULL;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd < 0) {
        if(error) *error = -2;
        return NULL;
    }

    memset(&serv_addr, 0, sizeof serv_addr);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = ip.s_addr;

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof serv_addr) < 0) {
        #ifdef __linux__
                close(sockfd);
        #else
                closesocket(sockfd);
        #endif
        if(error) *error = -3;
        return NULL;
    }

    #ifdef _WIN32
        blocking = 0;
        ioctlsocket(sockfd, FIONBIO, &blocking);
    #else
        int val = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, val | O_NONBLOCK);
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
    if (!socket_table[sockfd].writebuf)
        socket_table[sockfd].writebuf = malloc(WRITE_LENGTH);

    if (!socket_table[sockfd].writebuf) {
        #ifdef __linux__
                close(sockfd);
        #else
                closesocket(sockfd);
        #endif
        if(error) *error = -4;
        return NULL;
    }

    if (!socket_table[sockfd].rmsg)
        socket_table[sockfd].rmsg = malloc(MTU + 1);

    if (!socket_table[sockfd].rmsg) {
        if (socket_table[sockfd].writebuf) {
            free (socket_table[sockfd].writebuf);
            socket_table[sockfd].writebuf=NULL;
        }
        #ifdef __linux__
                close(sockfd);
        #else
                closesocket(sockfd);
        #endif
        if(error) *error = -5;
        return NULL;
    }


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

    gettimeofday(&now, NULL);
    tv = now;
    tv.tv_sec += SERVER_HANDSHAKE_TIMEOUT;

    if (!sched_add(socket_table+sockfd, SCHED_SERVER_HANDSHAKE_TIMEOUT, tv)) {
        closesock(socket_table+sockfd);
        if(error) *error = -6;
        return NULL;
    }



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


void disconnect_servers(void) {

    lattice_message mess;

    closeallservers();

    lattice_player.centeredon.x = lattice_player.wpos.x >> 8;
    lattice_player.centeredon.y = lattice_player.wpos.y >> 8;
    lattice_player.centeredon.z = lattice_player.wpos.z >> 8;
    lattice_player.my_min_wcoord.x = lattice_player.centeredon.x << 8;
    lattice_player.my_min_wcoord.y = lattice_player.centeredon.y << 8;
    lattice_player.my_min_wcoord.z = lattice_player.centeredon.z << 8;
    lattice_player.my_max_wcoord.x = (lattice_player.centeredon.x << 8) | 0x000000FF;
    lattice_player.my_max_wcoord.y = (lattice_player.centeredon.y << 8) | 0x000000FF;
    lattice_player.my_max_wcoord.z = (lattice_player.centeredon.z << 8) | 0x000000FF;

    mess.type = T_DISCONNECTED;
    ClrFlagFrom(&mess);
    mess.fromuid = 0;
    mess.length = 0;
    mess.args = NULL;

    (*gcallback)(&mess);

    return;

}

void recenter_neighbors(n_coord newcenter) {
    server_socket *s;
    int x;
    int y;
    int z;

    if (!find_neighbor(newcenter)) return;

    // get rid of the servers that are leaving us

    for (x=0;x<3;x++)
    for (y=0;y<3;y++)
    for (z=0;z<3;z++) {
        if ((s=neighbor_table[x][y][z])) {
            if (!serv_in_range_of_serv(newcenter, s->coord)) {
                // we are falling off the edge of the neighbor table
                sendto_one(s, "SLIDEOVER\n");
                neighbor_table[x][y][z] = NULL;
            }
        }
    }


    // neighbor table matrix translation

    if (serv_in_range_of_serv(lattice_player.centeredon, newcenter)) {

        // X plane

        if (newcenter.x > lattice_player.centeredon.x) {
            for (x=1;x<=2;x++) for (y=0;y<=2;y++) for (z=0;z<=2;z++) {
                neighbor_table[x-1][y][z] = neighbor_table[x][y][z];
                neighbor_table[x][y][z] = NULL;
            }
        }
        if (newcenter.x < lattice_player.centeredon.x) {
            for (x=1;x>=0;x--) for (y=2;y>=0;y--) for (z=2;z>=0;z--) {
                neighbor_table[x+1][y][z] = neighbor_table[x][y][z];
                neighbor_table[x][y][z] = NULL;
            }
        }

        // Y plane

        if (newcenter.y > lattice_player.centeredon.y) {
            for (x=0;x<=2;x++) for (y=1;y<=2;y++) for (z=0;z<=2;z++) {
                neighbor_table[x][y-1][z] = neighbor_table[x][y][z];
                neighbor_table[x][y][z] = NULL;
            }
        }
        if (newcenter.y < lattice_player.centeredon.y) {
            for (x=2;x>=0;x--) for (y=1;y>=0;y--) for (z=2;z>=0;z--) {
                neighbor_table[x][y+1][z] = neighbor_table[x][y][z];
                neighbor_table[x][y][z] = NULL;
            }
        }

        // Z plane

        if (newcenter.z > lattice_player.centeredon.z) {
            for (x=0;x<=2;x++) for (y=0;y<=2;y++) for (z=1;z<=2;z++) {
                neighbor_table[x][y][z-1] = neighbor_table[x][y][z];
                neighbor_table[x][y][z] = NULL;
            }
        }
        if (newcenter.z < lattice_player.centeredon.z) {
            for (x=2;x>=0;x--) for (y=2;y>=0;y--) for (z=1;z>=0;z--) {
                neighbor_table[x][y][z+1] = neighbor_table[x][y][z];
                neighbor_table[x][y][z] = NULL;
            }
        }
    }


    // send out CENTERED AND SIDED MOVEs

    for (x=0;x<3;x++)
    for (y=0;y<3;y++)
    for (z=0;z<3;z++) {
        if ((s=neighbor_table[x][y][z])) {
            if (s == neighbor_table[1][1][1]) {
                // this is a new center
                sendto_one(s,
                           //                     wx wy wz bx by bz  HEAD  HAND
                           "CENTEREDMOVE %u %u %u %u %u %u %d %d %d %d %d %d %d %d %u\n",
                           lattice_player.centeredon.x,
                           lattice_player.centeredon.y,
                           lattice_player.centeredon.z,
                           lattice_player.wpos.x,
                           lattice_player.wpos.y,
                           lattice_player.wpos.z,
                           lattice_player.bpos.x,
                           lattice_player.bpos.y,
                           lattice_player.bpos.z,
                           lattice_player.hrot.xrot,
                           lattice_player.hrot.yrot,
                           lattice_player.hhold.item_id,
                           lattice_player.hhold.item_type,
                           lattice_player.mining,
                           lattice_player.usercolor);
            } else {
                // this is a moved side
                if (user_is_within_outer_border(lattice_player.wpos, s->coord)) {
                    sendto_one(s,
                               //                  wx wy wz bx by bz  HEAD  HAND
                               "SIDEDMOVE %u %u %u %u %u %u %d %d %d %d %d %d %d %d %u\n",
                               newcenter.x,
                               newcenter.y,
                               newcenter.z,
                               lattice_player.wpos.x,
                               lattice_player.wpos.y,
                               lattice_player.wpos.z,
                               lattice_player.bpos.x,
                               lattice_player.bpos.y,
                               lattice_player.bpos.z,
                               lattice_player.hrot.xrot,
                               lattice_player.hrot.yrot,
                               lattice_player.hhold.item_id,
                               lattice_player.hhold.item_type,
                               lattice_player.mining,
                               lattice_player.usercolor);

                } else {
                    sendto_one(s,
                               "SIDEDMOVE %u %u %u\n",
                               newcenter.x,
                               newcenter.y,
                               newcenter.z);
                } // if (user_is_within_outer_border(lattice_player.wpos, s->coord))
            } // if (s == neighbor_table[1][1][1])
        } // if ((s=neighbor_table[x][y][z]))
    } // for ()

    // update lattice_player
    lattice_player.centeredon.x = neighbor_table[1][1][1]->coord.x;
    lattice_player.centeredon.y = neighbor_table[1][1][1]->coord.y;
    lattice_player.centeredon.z = neighbor_table[1][1][1]->coord.z;
    lattice_player.my_min_wcoord.x = lattice_player.centeredon.x << 8;
    lattice_player.my_min_wcoord.y = lattice_player.centeredon.y << 8;
    lattice_player.my_min_wcoord.z = lattice_player.centeredon.z << 8;
    lattice_player.my_max_wcoord.x = (lattice_player.centeredon.x << 8) | 0x000000FF;
    lattice_player.my_max_wcoord.y = (lattice_player.centeredon.y << 8) | 0x000000FF;
    lattice_player.my_max_wcoord.z = (lattice_player.centeredon.z << 8) | 0x000000FF;

    return;

}


void packet_recenter_neighbors(n_coord newcenter) {
    server_socket *s;
    int x;
    int y;
    int z;

    void *p;
    lt_packet out_packet;

    if (!find_neighbor(newcenter)) return;

    // get rid of the servers that are leaving us

    for (x=0;x<3;x++)
    for (y=0;y<3;y++)
    for (z=0;z<3;z++) {
        if ((s=neighbor_table[x][y][z])) {
            if (!serv_in_range_of_serv(newcenter, s->coord)) {
                // we are falling off the edge of the neighbor table

                makepacket(&out_packet, T_SLIDEOVER);
                sendpacket(s, &out_packet);
                //sendto_one(s, "SLIDEOVER\n");

                neighbor_table[x][y][z] = NULL;
            }
        }
    }


    // neighbor table matrix translation

    if (serv_in_range_of_serv(lattice_player.centeredon, newcenter)) {

        // X plane

        if (newcenter.x > lattice_player.centeredon.x) {
            for (x=1;x<=2;x++) for (y=0;y<=2;y++) for (z=0;z<=2;z++) {
                neighbor_table[x-1][y][z] = neighbor_table[x][y][z];
                neighbor_table[x][y][z] = NULL;
            }
        }
        if (newcenter.x < lattice_player.centeredon.x) {
            for (x=1;x>=0;x--) for (y=2;y>=0;y--) for (z=2;z>=0;z--) {
                neighbor_table[x+1][y][z] = neighbor_table[x][y][z];
                neighbor_table[x][y][z] = NULL;
            }
        }

        // Y plane

        if (newcenter.y > lattice_player.centeredon.y) {
            for (x=0;x<=2;x++) for (y=1;y<=2;y++) for (z=0;z<=2;z++) {
                neighbor_table[x][y-1][z] = neighbor_table[x][y][z];
                neighbor_table[x][y][z] = NULL;
            }
        }
        if (newcenter.y < lattice_player.centeredon.y) {
            for (x=2;x>=0;x--) for (y=1;y>=0;y--) for (z=2;z>=0;z--) {
                neighbor_table[x][y+1][z] = neighbor_table[x][y][z];
                neighbor_table[x][y][z] = NULL;
            }
        }

        // Z plane

        if (newcenter.z > lattice_player.centeredon.z) {
            for (x=0;x<=2;x++) for (y=0;y<=2;y++) for (z=1;z<=2;z++) {
                neighbor_table[x][y][z-1] = neighbor_table[x][y][z];
                neighbor_table[x][y][z] = NULL;
            }
        }
        if (newcenter.z < lattice_player.centeredon.z) {
            for (x=2;x>=0;x--) for (y=2;y>=0;y--) for (z=1;z>=0;z--) {
                neighbor_table[x][y][z+1] = neighbor_table[x][y][z];
                neighbor_table[x][y][z] = NULL;
            }
        }
    }


    // send out CENTERED AND SIDED MOVEs

    for (x=0;x<3;x++)
    for (y=0;y<3;y++)
    for (z=0;z<3;z++) {
        if ((s=neighbor_table[x][y][z])) {
            if (s == neighbor_table[1][1][1]) {
                // this is a new center
                makepacket(&out_packet, T_CENTEREDMOVE);
                p = &out_packet.payload;
                put_nx(&p, lattice_player.centeredon.x, &PLength(&out_packet), &PArgc(&out_packet));
                put_ny(&p, lattice_player.centeredon.y, &PLength(&out_packet), &PArgc(&out_packet));
                put_nz(&p, lattice_player.centeredon.z, &PLength(&out_packet), &PArgc(&out_packet));
                put_wx(&p, lattice_player.wpos.x, &PLength(&out_packet), &PArgc(&out_packet));
                put_wy(&p, lattice_player.wpos.y, &PLength(&out_packet), &PArgc(&out_packet));
                put_wz(&p, lattice_player.wpos.z, &PLength(&out_packet), &PArgc(&out_packet));
                put_bx(&p, lattice_player.bpos.x, &PLength(&out_packet), &PArgc(&out_packet));
                put_by(&p, lattice_player.bpos.y, &PLength(&out_packet), &PArgc(&out_packet));
                put_bz(&p, lattice_player.bpos.z, &PLength(&out_packet), &PArgc(&out_packet));
                put_xrot(&p, lattice_player.hrot.xrot, &PLength(&out_packet), &PArgc(&out_packet));
                put_yrot(&p, lattice_player.hrot.yrot, &PLength(&out_packet), &PArgc(&out_packet));
                put_item_id(&p, lattice_player.hhold.item_id, &PLength(&out_packet), &PArgc(&out_packet));
                put_item_type(&p, lattice_player.hhold.item_type, &PLength(&out_packet), &PArgc(&out_packet));
                put_mining(&p, lattice_player.mining, &PLength(&out_packet), &PArgc(&out_packet));
                put_usercolor(&p, lattice_player.usercolor, &PLength(&out_packet), &PArgc(&out_packet));
                sendpacket(s, &out_packet);

                //sendto_one(s,
                //           //                     wx wy wz bx by bz  HEAD  HAND
                //           "CENTEREDMOVE %u %u %u %u %u %u %d %d %d %d %d %d %d %d %u\n",
                //           lattice_player.centeredon.x,
                //           lattice_player.centeredon.y,
                //           lattice_player.centeredon.z,
                //           lattice_player.wpos.x,
                //           lattice_player.wpos.y,
                //           lattice_player.wpos.z,
                //           lattice_player.bpos.x,
                //           lattice_player.bpos.y,
                //           lattice_player.bpos.z,
                //           lattice_player.hrot.xrot,
                //           lattice_player.hrot.yrot,
                //           lattice_player.hhold.item_id,
                //           lattice_player.hhold.item_type,
                //           lattice_player.mining,
                //           lattice_player.usercolor);
            } else {
                // this is a moved side
                if (user_is_within_outer_border(lattice_player.wpos, s->coord)) {

                    makepacket(&out_packet, T_SIDEDMOVE);
                    p = &out_packet.payload;
                    put_nx(&p, newcenter.x, &PLength(&out_packet), &PArgc(&out_packet));
                    put_ny(&p, newcenter.y, &PLength(&out_packet), &PArgc(&out_packet));
                    put_nz(&p, newcenter.z, &PLength(&out_packet), &PArgc(&out_packet));
                    put_wx(&p, lattice_player.wpos.x, &PLength(&out_packet), &PArgc(&out_packet));
                    put_wy(&p, lattice_player.wpos.y, &PLength(&out_packet), &PArgc(&out_packet));
                    put_wz(&p, lattice_player.wpos.z, &PLength(&out_packet), &PArgc(&out_packet));
                    put_bx(&p, lattice_player.bpos.x, &PLength(&out_packet), &PArgc(&out_packet));
                    put_by(&p, lattice_player.bpos.y, &PLength(&out_packet), &PArgc(&out_packet));
                    put_bz(&p, lattice_player.bpos.z, &PLength(&out_packet), &PArgc(&out_packet));
                    put_xrot(&p, lattice_player.hrot.xrot, &PLength(&out_packet), &PArgc(&out_packet));
                    put_yrot(&p, lattice_player.hrot.yrot, &PLength(&out_packet), &PArgc(&out_packet));
                    put_item_id(&p, lattice_player.hhold.item_id, &PLength(&out_packet), &PArgc(&out_packet));
                    put_item_type(&p, lattice_player.hhold.item_type, &PLength(&out_packet), &PArgc(&out_packet));
                    put_mining(&p, lattice_player.mining, &PLength(&out_packet), &PArgc(&out_packet));
                    put_usercolor(&p, lattice_player.usercolor, &PLength(&out_packet), &PArgc(&out_packet));
                    sendpacket(s, &out_packet);

                    //sendto_one(s,
                    //           //                  wx wy wz bx by bz  HEAD  HAND
                    //           "SIDEDMOVE %u %u %u %u %u %u %d %d %d %d %d %d %d %d %u\n",
                    //           newcenter.x,
                    //           newcenter.y,
                    //           newcenter.z,
                    //           lattice_player.wpos.x,
                    //           lattice_player.wpos.y,
                    //           lattice_player.wpos.z,
                    //           lattice_player.bpos.x,
                    //           lattice_player.bpos.y,
                    //           lattice_player.bpos.z,
                    //           lattice_player.hrot.xrot,
                    //           lattice_player.hrot.yrot,
                    //           lattice_player.hhold.item_id,
                    //           lattice_player.hhold.item_type,
                    //           lattice_player.mining,
                    //           lattice_player.usercolor);

                } else {
                    makepacket(&out_packet, T_SIDEDMOVE);
                    p = &out_packet.payload;
                    put_nx(&p, newcenter.x, &PLength(&out_packet), &PArgc(&out_packet));
                    put_ny(&p, newcenter.y, &PLength(&out_packet), &PArgc(&out_packet));
                    put_nz(&p, newcenter.z, &PLength(&out_packet), &PArgc(&out_packet));
                    sendpacket(s, &out_packet);

                    //sendto_one(s,
                    //           "SIDEDMOVE %u %u %u\n",
                    //           newcenter.x,
                    //           newcenter.y,
                    //           newcenter.z);
                } // if (user_is_within_outer_border(lattice_player.wpos, s->coord))
            } // if (s == neighbor_table[1][1][1])
        } // if ((s=neighbor_table[x][y][z]))
    } // for ()

    // update lattice_player
    lattice_player.centeredon.x = neighbor_table[1][1][1]->coord.x;
    lattice_player.centeredon.y = neighbor_table[1][1][1]->coord.y;
    lattice_player.centeredon.z = neighbor_table[1][1][1]->coord.z;
    lattice_player.my_min_wcoord.x = lattice_player.centeredon.x << 8;
    lattice_player.my_min_wcoord.y = lattice_player.centeredon.y << 8;
    lattice_player.my_min_wcoord.z = lattice_player.centeredon.z << 8;
    lattice_player.my_max_wcoord.x = (lattice_player.centeredon.x << 8) | 0x000000FF;
    lattice_player.my_max_wcoord.y = (lattice_player.centeredon.y << 8) | 0x000000FF;
    lattice_player.my_max_wcoord.z = (lattice_player.centeredon.z << 8) | 0x000000FF;

    return;

}
