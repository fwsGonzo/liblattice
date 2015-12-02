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

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __linux__
#include <sys/time.h>
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>
#include <stdio.h>
#else
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#ifndef __MINGW32__
#include "forwin.h"
#else
#include <sys/time.h>
#endif
#endif

#include "lattice_config.h"
#include "serversocket.h"
#include "struct.h"
#include "send.h"
#include "globals.h"
#include "macros.h"
#include "lattice_packet.h"

void sendclient_emptysector(w_coord wcoord) {

    lattice_message mess;
    lattice_emptysector submess;

    mess.type = T_EMPTYSECTOR;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.length = sizeof submess;
    mess.args = &submess;

    submess.wcoord.x = wcoord.x;
    submess.wcoord.y = wcoord.y;
    submess.wcoord.z = wcoord.z;

    (*gcallback)(&mess);

    return;

}

void sendclient_emptyflatland(f_coord fcoord) {

    lattice_message mess;
    lattice_flatland submess;

    mess.type = T_FLATLAND;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.length = sizeof submess;
    mess.args = &submess;

    submess.fcoord.x = fcoord.x;
    submess.fcoord.z = fcoord.z;

    memset(submess.fdata, 0, sizeof submess.fdata);

    (*gcallback)(&mess);

    return;

}

void sendclient_emptycolumn(f_coord fcoord) {

    w_coord wcoord;

    sendclient_emptyflatland(fcoord);

    wcoord.x = fcoord.x;
    wcoord.z = fcoord.z;

    for (wcoord.y = 0; wcoord.y < BLOCKSDB_COUNT_WY; wcoord.y++)
        sendclient_emptysector(wcoord);

    return;

}

#define can_burst_from(fcoord) (                                                        \
                                ( ((fcoord).x >= bot.x) && ((fcoord).x <= top.x) ) &&   \
                                ( ((fcoord).z >= bot.z) && ((fcoord).z <= top.z) )      \
                               )

void burstclient_emptysectors(n_coord coord, int burstdist, w_coord wcoord) {
    w_coord min_wcoord;
    w_coord max_wcoord;

    w_coord bot;
    w_coord top;

    //w_coord scroll;

    f_coord fcoord;

    int32_t i;
    int32_t n;
    int32_t max;

    min_wcoord.x = coord.x << BLOCKSDB_WIDTH_WX;
    min_wcoord.y = coord.y << BLOCKSDB_WIDTH_WY;
    min_wcoord.z = coord.z << BLOCKSDB_WIDTH_WX;
    max_wcoord.x = (coord.x << BLOCKSDB_WIDTH_WX) | BLOCKSDB_MASK_WX;
    max_wcoord.y = (coord.y << BLOCKSDB_WIDTH_WY) | BLOCKSDB_MASK_WY;
    max_wcoord.z = (coord.z << BLOCKSDB_WIDTH_WZ) | BLOCKSDB_MASK_WZ;

    if (wcoord.x - burstdist < min_wcoord.x) bot.x = min_wcoord.x; else bot.x = wcoord.x - burstdist;
    if (wcoord.y - burstdist < min_wcoord.y) bot.y = min_wcoord.y; else bot.y = wcoord.y - burstdist;
    if (wcoord.z - burstdist < min_wcoord.z) bot.z = min_wcoord.z; else bot.z = wcoord.z - burstdist;

    if (wcoord.x - burstdist < min_wcoord.x) bot.x = min_wcoord.x; else bot.x = wcoord.x - burstdist;
    if (wcoord.y - burstdist < min_wcoord.y) bot.y = min_wcoord.y; else bot.y = wcoord.y - burstdist;
    if (wcoord.z - burstdist < min_wcoord.z) bot.z = min_wcoord.z; else bot.z = wcoord.z - burstdist;

    if (wcoord.x + burstdist > max_wcoord.x) top.x = max_wcoord.x; else top.x = wcoord.x + burstdist;
    if (wcoord.y + burstdist > max_wcoord.y) top.y = max_wcoord.y; else top.y = wcoord.y + burstdist;
    if (wcoord.z + burstdist > max_wcoord.z) top.z = max_wcoord.z; else top.z = wcoord.z + burstdist;

    max = burstdist;

    fcoord.x = wcoord.x;
    fcoord.z = wcoord.z;

    if (can_burst_from(fcoord)) {
        sendclient_emptycolumn(fcoord);
    }

    for (n = 1; n <= max; n++) {

        for (i = -n; i <= n; i++) {
            fcoord.x = wcoord.x - n;
            fcoord.z = wcoord.z + i;
            if (can_burst_from(fcoord))
                sendclient_emptycolumn(fcoord);
        }

        for (i = (-n + 1); i < n; i++) {
            fcoord.x = wcoord.x + i;
            fcoord.z = wcoord.z - n;
            if (can_burst_from(fcoord))
                sendclient_emptycolumn(fcoord);
            fcoord.x = wcoord.x + i;
            fcoord.z = wcoord.z + n;
            if (can_burst_from(fcoord))
                sendclient_emptycolumn(fcoord);
        }

        for (i = -n; i <= n; i++) {
            fcoord.x = wcoord.x + n;
            fcoord.z = wcoord.z + i;
            if (can_burst_from(fcoord))
                sendclient_emptycolumn(fcoord);
        }

    }

    return;

}

