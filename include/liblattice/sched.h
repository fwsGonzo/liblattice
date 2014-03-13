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


extern int sched_server_handshake_timeout(server_socket *socket);
extern int sched_server_send_ping(server_socket *socket);
extern int sched_server_ping_timeout(server_socket *socket);

extern sched_usec_link *sched_add_usec_link(sched_usec_header *header, long usec, int type, server_socket *socket);
extern void sched_del_usec_link(sched_usec_header *header, sched_usec_link *link);
extern sched_usec_link *sched_find_usec_link(sched_usec_header *header, long usec);
extern sched_usec_link *sched_rfind_usec_link(sched_usec_header *header, long usec);

extern sched_sec_link *sched_add_sec_link(sched_header *header, long sec, int *ran_malloc);
extern void sched_del_sec_link(sched_header *header, sched_sec_link *link);

extern sched_sec_link *sched_find_sec_link(sched_header *header, long sec);
extern sched_sec_link *sched_rfind_sec_link(sched_header *header, long sec);

extern int sched_add(server_socket *socket, int type, struct timeval triggr_time);
extern struct timeval *sched_whens_next(sched_header *header, struct timeval *t);
extern void sched_delfrom(sched_header *header, server_socket *socket);
extern void sched_deltypefrom(sched_header *header, server_socket *socket, int type);
extern int sched_process(int type, server_socket *socket);
extern void sched_process_all(sched_header *header);

