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
#include <errno.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "lattice_config.h"
#include "serversocket.h"
#include "struct.h"
#include "globals.h"
#include "send.h"
#include "macros.h"
#include "socket.h"

int sched_add(server_socket *socket, int type, struct timeval triggr_time);

// ----------------------------------------------------------------------



// -----------------------------------------


sched_usec_link *sched_add_usec_link(sched_usec_header *header, long usec, int type, server_socket *socket) {

    sched_usec_link *newlink;
    sched_usec_link *p;

    if (!header) return NULL;

    newlink = malloc(sizeof(sched_usec_link));

    if (!newlink) return NULL;

    newlink->usec = usec;
    newlink->type = type;
    newlink->socket = socket;

    if (!header->head) { // empty list
        header->head = header->tail = newlink;
        newlink->prev = newlink->next = NULL;
        return newlink;
    }

    for (p=header->head; p && p->usec <= usec; p=p->next);

    if (!p) { // insert at the end
        newlink->prev = header->tail;
        newlink->next = NULL;
        header->tail->next = newlink;
        header->tail = newlink;
    } else { // insert directly before p
        if (p == header->head) {
            // were at the head of the list
            newlink->next = header->head;
            newlink->prev = NULL;
            header->head->prev = newlink;
            header->head = newlink;
        } else {
            // were somewhere in the middle
            newlink->prev = p->prev;
            newlink->next = p;
            p->prev->next = newlink;
            p->prev = newlink;
        }

    }

    return newlink;
}


void sched_del_usec_link(sched_usec_header *header, sched_usec_link *link) {
    if (!header || !link) return;

    if (link->prev) link->prev->next = link->next; else header->head = link->next;
    if (link->next) link->next->prev = link->prev; else header->tail = link->prev;

    free(link);

    return;
}

sched_usec_link *sched_find_usec_link(sched_usec_header *header, long usec) {
    sched_usec_link *p;
    if (!header) return NULL;
    for (p = header->head; p; p = p->next)
        if (p->usec == usec)
            return p;

    return NULL;
}

sched_usec_link *sched_rfind_usec_link(sched_usec_header *header, long usec) {
    sched_usec_link *p;
    if (!header) return NULL;
    for (p = header->tail; p; p = p->prev)
        if (p->usec == usec)
            return p;

    return NULL;
}

sched_sec_link *sched_add_sec_link(sched_header *header, long sec, int *ran_malloc) {

    sched_sec_link *newlink;
    sched_sec_link *p;

    if (!header) {
        if (ran_malloc) *ran_malloc = 0;
        return NULL;
    }

    if (!header->head) { // empty list
        newlink = malloc(sizeof(sched_sec_link));
        if (!newlink) {
            if (ran_malloc) *ran_malloc = 0;
            return NULL;
        }
        newlink->sec = sec;
        newlink->usec_header.head = NULL;
        newlink->usec_header.tail = NULL;
        header->head = header->tail = newlink;
        newlink->prev = newlink->next = NULL;
        if (ran_malloc) *ran_malloc = 1;
        return newlink;
    }

    for (p=header->head; p && p->sec < sec; p=p->next);

    if (p && p->sec == sec) { // we found sec
        if (ran_malloc) *ran_malloc = 0;
        return p;
    }

    newlink = malloc(sizeof(sched_sec_link));

    if (!newlink) {
        if (ran_malloc) *ran_malloc = 0;
        return NULL;
    }

    newlink->sec = sec;
    newlink->usec_header.head = NULL;
    newlink->usec_header.tail = NULL;

    if (!p) { // insert at the end
        newlink->prev = header->tail;
        newlink->next = NULL;
        header->tail->next = newlink;
        header->tail = newlink;
    } else { // insert directly before p
        if (p == header->head) {
            // were at the head of the list
            newlink->next = header->head;
            newlink->prev = NULL;
            header->head->prev = newlink;
            header->head = newlink;
        } else {
            // were somewhere in the middle
            newlink->prev = p->prev;
            newlink->next = p;
            p->prev->next = newlink;
            p->prev = newlink;
        }

    }

    if (ran_malloc) *ran_malloc = 1;
    return newlink;
}

void sched_del_sec_link(sched_header *header, sched_sec_link *link) {
    if (!header || !link) return;

    if (link->prev) link->prev->next = link->next; else header->head = link->next;
    if (link->next) link->next->prev = link->prev; else header->tail = link->prev;

    free(link);

    return;
}

sched_sec_link *sched_find_sec_link(sched_header *header, long sec) {
    sched_sec_link *p;
    if (!header) return NULL;
    for (p = header->head; p; p = p->next)
        if (p->sec == sec)
            return p;

    return NULL;
}

sched_sec_link *sched_rfind_sec_link(sched_header *header, long sec) {
    sched_sec_link *p;
    if (!header) return NULL;
    for (p = header->tail; p; p = p->prev)
        if (p->sec == sec)
            return p;

    return NULL;
}

// 0 on fail
int sched_add(server_socket *socket, int type, struct timeval triggr_time) {

    sched_sec_link  *p;
    sched_usec_link *q;

    int ran_malloc;

    p = sched_add_sec_link(&sched, triggr_time.tv_sec, &ran_malloc);

    if (!p) return 0;

    q = sched_add_usec_link(&p->usec_header, triggr_time.tv_usec, type, socket);

    if (!q) {
        if (ran_malloc) sched_del_sec_link(&sched, p);
        return 0;
    }

    return 1;
}

struct timeval *sched_whens_next(sched_header *header, struct timeval *t) {

    struct timeval temp;

    if (!header || !t) return NULL; // busted call

    if (!header->head) return NULL; // nothing scheduled

    if (!header->head->usec_header.head) return NULL; // no usec chain?

    temp.tv_sec = header->head->sec;
    temp.tv_usec = header->head->usec_header.head->usec;

    if (!timercmp(&temp, &now, >)) { // event is in past or now
        timerclear(t);
        return t;
    }

    timersub(&temp, &now, t);
    return t;

}

void sched_delfrom(sched_header *header, server_socket *socket) {

    sched_sec_link *sec;
    sched_usec_link *usec;
    sched_sec_link *holdsec;
    sched_usec_link *holdusec;

    if (!header || !socket) return;

    sec = header->head;
    while (sec) {

        usec = sec->usec_header.head;
        while (usec) {
            if (usec->socket == socket) {
                holdusec = usec->next;
                sched_del_usec_link(&sec->usec_header, usec);
                usec = holdusec;
                continue;
            }
            usec = usec->next;
        }
        if (!sec->usec_header.head) {
            holdsec = sec->next;
            sched_del_sec_link(header, sec);
            sec = holdsec;
            continue;
        }
        sec = sec->next;
    }

    return;
}

void sched_deltypefrom(sched_header *header, server_socket *socket, int type) {

    sched_sec_link *sec;
    sched_usec_link *usec;
    sched_sec_link *holdsec;
    sched_usec_link *holdusec;

    if (!header || !socket) return;

    sec = header->head;
    while (sec) {

        usec = sec->usec_header.head;
        while (usec) {
            if (usec->socket == socket &&
                usec->type == type) {
                holdusec = usec->next;
                sched_del_usec_link(&sec->usec_header, usec);
                usec = holdusec;
                continue;
            }
            usec = usec->next;
        }
        if (!sec->usec_header.head) {
            holdsec = sec->next;
            sched_del_sec_link(header, sec);
            sec = holdsec;
            continue;
        }
        sec = sec->next;
    }

    return;
}


int sched_process(int type, server_socket *socket) {
    int i;
    //if (!socket) return 0; // nto all events are tied to a socket (eg blocksautosave)

    switch(type) {
        default:
            i=0;
        break;
    }

    return i;
}

void sched_process_all(sched_header *header) {

    sched_sec_link *sec;
    sched_usec_link *usec;
    sched_sec_link *holdsec;
    sched_usec_link *holdusec;

    struct timeval tv;

    int have_to_restart=0;

    if (!header) return;

    do {
        sec = header->head;
        while (sec && sec->sec <= now.tv_sec) {
            tv.tv_sec = sec->sec;
            usec = sec->usec_header.head;
            while (usec) {
                tv.tv_usec = usec->usec;
                if (!timercmp(&tv, &now, >)) {
                    holdusec = usec->next;
                    have_to_restart=sched_process(usec->type, usec->socket);
                    if (have_to_restart) break;

                    sched_del_usec_link(&sec->usec_header, usec);
                    usec = holdusec;
                    continue;
                } else break;
                usec = usec->next;
            }
            if (have_to_restart) { have_to_restart = 0; break; }
            if (!sec->usec_header.head) {
                holdsec = sec->next;
                sched_del_sec_link(header, sec);
                sec = holdsec;
                continue;
            }
            sec = sec->next;
        }

    } while (have_to_restart);

    return;
}


// ------------------------
