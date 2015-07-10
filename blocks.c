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
