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



#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#ifdef __linux__
#include <getopt.h>
#include <unistd.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <errno.h>
#else
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#ifdef __MINGW32__
#include <getopt.h>
#include <unistd.h>
#endif
#endif

#include "lattice_config.h"
#include "serversocket.h"
#include "struct.h"
#include "globals.h"
#include "socket.h"
#include "macros.h"
//#include "numerics.h"
#include "lattice_packet.h"

#define IsWritable(fd) (                                                                                    \
    (                                                                                                       \
        ( FD_ISSET((fd), &rtest_set) && !FD_ISSET((fd), &wtest_set) ) ||                                    \
        ( FD_ISSET((fd), &rtest_set) &&  FD_ISSET((fd), &wtest_set) && FD_ISSET((fd), &wready_set) )        \
    )                                                                                                       \
)


// -1 on error
int flush_write(server_socket *s, int closing) {

    int ret;

    if (!s) return(-1);
    if (s->socket < 0) return(-1);
    if (!FD_ISSET(s->socket, &rtest_set) && !FD_ISSET(s->socket, &wtest_set)) return(-1);
    if (s->wlen <= 0) return(-1);

    if (!IsWritable(s->socket)) return(0);

    //ret = write(s->socket, s->writebuf, s->wlen);
    ret = send(s->socket, s->writebuf, s->wlen, 0);

    if (ret < 0) {
        if (!sock_ignoreError(sock_getError()) && !closing) {
            SetFlagClosed(s);
            closesock(s);
            return(-1);
        }
        ret=0;
    }
    if (!ret) return(0);

    if (ret < s->wlen) { // partial write! slide the rest over!
        memmove(s->writebuf, s->writebuf + ret, s->wlen - ret);
        if (!FD_ISSET(s->socket, &wtest_set))
            FD_SET(s->socket, &wtest_set);
    } else {
        if (!s->sendq_head) {
            if (FD_ISSET(s->socket, &wtest_set))
                FD_CLR(s->socket, &wtest_set);
        }
    }

    s->wlen -= ret;

    return(ret);

}

void flushall_write(void) {

    int fd;

    for (fd = 0; fd <= maxfd; fd++)
        if ( IsWritable(fd) )
                flush_write(socket_table+fd, 0);

    return;

}

// 0 on fail
int sendq_add(server_socket *s, void *data, int length) {

    size_t sendq_move_length;
    size_t sendq_read_index;

    sendq_link *new_sendq_link;

    if (!s || !data) return(0);
    if (length < 0) return(0);
    if (!length) return(1);

    sendq_read_index = 0;

    // sendq overflow!
    if (s->sendq_length + length > s->sendq_max) return(0);

    if (!s->sendq_head) {
        // no sendq exists!

        new_sendq_link = malloc(sizeof(sendq_link));
        if (!new_sendq_link) return(0);
        new_sendq_link->length = 0;
        new_sendq_link->offset = 0;
        new_sendq_link->next = NULL;

        s->sendq_head = s->sendq_tail = new_sendq_link;
        s->sendq_length = 0;
    }

    while (sendq_read_index < length) {

        // check length of input remaining VS length left in this sendq block

        sendq_move_length = my_min(length - sendq_read_index,
            SENDQ_BLOCK - s->sendq_tail->length);

        if (sendq_move_length > 0) {
            memcpy(s->sendq_tail->sendq_block + s->sendq_tail->length,
                (char *) data + sendq_read_index,
                sendq_move_length);

            sendq_read_index += sendq_move_length;
            s->sendq_tail->length += sendq_move_length;
            s->sendq_length += sendq_move_length;
        }

        if (sendq_read_index < length) {

            // we still have more to add in the sendq...we need a new block

            new_sendq_link = malloc(sizeof(sendq_link));

            if (!new_sendq_link) return(0);
            new_sendq_link->length = 0;
            new_sendq_link->offset = 0;
            new_sendq_link->next = NULL;

            s->sendq_tail->next = new_sendq_link;
            s->sendq_tail = new_sendq_link;

        }

    } // End while (sendq_read_index < length)

    return(1);

}

// 0 on fail
int sendq_pop(server_socket *s, int length) {

    int sendq_pop_index=0;
    int sendq_pop_willing;
    int sendq_pop_move;
    sendq_link *sendq_next;

    if (!s) return(0);

    // trunc length to sendq length
    sendq_pop_willing = my_min(length, s->sendq_length);

    while (sendq_pop_index < sendq_pop_willing) {

        // length of remaining head sendq block VS how much remains to remove

        sendq_pop_move = my_min (
            s->sendq_head->length - s->sendq_head->offset,
            sendq_pop_willing - sendq_pop_index);

        if (sendq_pop_move > 0) {
            s->sendq_head->offset += sendq_pop_move;
            sendq_pop_index += sendq_pop_move;
            s->sendq_length -= sendq_pop_move;
        }

        if (s->sendq_head->length == s->sendq_head->offset) {

            // we pulled everything out of this block... remove it.

            sendq_next = s->sendq_head->next;
            free(s->sendq_head);
            s->sendq_head = sendq_next;
            if (!s->sendq_head) s->sendq_tail=NULL;
        }

    }

    return(1);
}

// 0 on fail


//int sendq_popall(server_socket *s)
//
// this can be found in macro.h

// 0 on fail (theres more in the sendq)
// 1 sendq is now empty
int sendq_flush(server_socket *s) {

    int sendq_flush_move;
    int sendq_flush_index=0;
    int sendq_flush_willing;
    sendq_link *sendq_next;

    if (!s->sendq_head) return(1);
    //if (!TestSocketLive(s)) return(1);
    //if (TestSocketClosed(s)) return(1);



    // trunc write block remaining length to sendq length
    sendq_flush_willing = my_min(
        WRITE_LENGTH - s->wlen,
        s->sendq_length);

    while (sendq_flush_index < sendq_flush_willing) {

        // length of remaining head sendq block VS how much remains to remove

        sendq_flush_move = my_min (
            s->sendq_head->length - s->sendq_head->offset,
            sendq_flush_willing - sendq_flush_index);

        if (sendq_flush_move > 0) {
            memcpy(s->writebuf + s->wlen,
                s->sendq_head->sendq_block + s->sendq_head->offset,
                sendq_flush_move);

            s->sendq_head->offset += sendq_flush_move;
            s->wlen += sendq_flush_move;
            sendq_flush_index += sendq_flush_move;
            s->sendq_length -= sendq_flush_move;
        }

        if (s->sendq_head->length == s->sendq_head->offset) {

            // we pulled everything out of this block... remove it.

            sendq_next = s->sendq_head->next;
            free(s->sendq_head);
            s->sendq_head = sendq_next;
            if (!s->sendq_head) s->sendq_tail=NULL;
        }


    }

    if (s->sendq_length > 0)
        return(0);
    else
        return(1);
}


// 1 if we closed the socket
int writeto(server_socket *s, const void *buf, size_t count) {

    int writeto_move;

    if (!s || !buf) return 0;
    if (count <= 0) return 0;


    // if theres already a sendq we must append...

    if (s->sendq_head) {
        if (sendq_add(s, (char *) buf, count)) {
            if (!FD_ISSET(s->socket, &wtest_set))
                FD_SET(s->socket, &wtest_set);
        } else {
            SetFlagClosed(s);
            closesock(s);
            return 1;
        }
        return 0;
    }

    // length we are attempting to write VS length left in sockets write_block

    writeto_move = my_min(count, WRITE_LENGTH - s->wlen);

    if (writeto_move > 0) {
        memcpy(s->writebuf + s->wlen,
            buf,
            writeto_move);

        s->wlen += writeto_move;
    }

    if (writeto_move < count) {
        if (sendq_add(s, (char *) buf + writeto_move, count - writeto_move)) {
            if (!FD_ISSET(s->socket, &wtest_set))
                FD_SET(s->socket, &wtest_set);
        } else {
            SetFlagClosed(s);
            closesock(s);
            return 1;
        }

    }

    return 0;

}


//  -------- sendto stuff

int sendto_one(server_socket *entry, char *format, ...) {

    char buf[8192];

    int i;
    int ret=0;

    va_list args;

    va_start(args, format);

    i=vsnprintf((char *)buf, 8192, format, args);

    if (i > 0)
        ret=writeto(entry, (char *) buf, i);

    va_end(args);

    return ret;
}


int sendto_allservers(char *format, ...) {

    char buf[8192];
    server_socket *s;
    int x;
    int y;
    int z;
    int i;

    va_list args;

    if (!format) return 0;

    va_start(args, format);

    i=vsnprintf((char *)buf, 8192, format, args);

    if (i > 0)
    for (x=0;x<3;x++)
    for (y=0;y<3;y++)
    for (z=0;z<3;z++) {
        if ((s=neighbor_table[x][y][z])) {
            writeto(s, (char *) buf, i);
        }
    }


    va_end(args);

    return(i);
}



int sendto_allservers_butone(server_socket *butme, char *format, ...) {

    char buf[8192];
    server_socket *s;
    int x;
    int y;
    int z;
    int i;

    va_list args;

    if (!butme || !format) return 0;

    va_start(args, format);

    i=vsnprintf((char *)buf, 8192, format, args);

    if (i > 0)
    for (x=0;x<3;x++)
    for (y=0;y<3;y++)
    for (z=0;z<3;z++) {
        if ((s=neighbor_table[x][y][z])) {
              if (s != butme)
                writeto(s, (char *) buf, i);
        }
    }


    va_end(args);

    return(i);
}



// packet header is passed in HOST order

int sendpacket(server_socket *entry, lt_packet *packet) {

    uint16_t packet_length;

    int ret=0;

    if (!entry || !packet) return 0;

    packet_length = packet->header.payload_length + sizeof(lt_packet_h);

    packet_hton(&packet->header);

    ret=writeto(entry, packet, packet_length);

    packet_ntoh(&packet->header);

    return ret;

}

int sendpacketto_allservers(lt_packet *packet) {

    uint16_t packet_length;
    server_socket *s;
    int x;
    int y;
    int z;

    if (!packet) return 0;

    packet_length = packet->header.payload_length + sizeof(lt_packet_h);

    packet_hton(&packet->header);

    for (x=0;x<3;x++)
    for (y=0;y<3;y++)
    for (z=0;z<3;z++) {
        if ((s=neighbor_table[x][y][z])) {
            writeto(s, packet, packet_length);
        }
    }

    packet_ntoh(&packet->header);

    return(packet_length);
}

int sendpacketto_allservers_butone(server_socket *butme, lt_packet *packet) {

    uint16_t packet_length;
    server_socket *s;
    int x;
    int y;
    int z;

    if (!butme || !packet) return 0;

    packet_length = packet->header.payload_length + sizeof(lt_packet_h);

    packet_hton(&packet->header);

    for (x=0;x<3;x++)
    for (y=0;y<3;y++)
    for (z=0;z<3;z++) {
        if ((s=neighbor_table[x][y][z])) {
            if (s != butme)
                writeto(s, packet, packet_length);
        }
    }

    packet_ntoh(&packet->header);

    return(packet_length);
}
