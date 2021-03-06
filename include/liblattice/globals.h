#ifndef GLOBALS_H
#define GLOBALS_H

#include "serversocket.h"

extern server_socket *socket_table;

extern server_socket *neighbor_table[3][3][3];

extern fd_set rready_set;
extern fd_set wready_set;
extern fd_set rtest_set;
extern fd_set wtest_set;

extern int input_sock;

extern int maxfd;

extern struct timeval now;

extern int reach;

extern lattice_player_t lattice_player;

extern void (*gcallback)(lattice_message *mp);

extern sched_header sched;

extern int lattice_initialized;

extern int lattice_connected;

#endif
