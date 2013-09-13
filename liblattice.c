#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#ifdef __linux__
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <errno.h>
	#include <sys/resource.h>
#else
    #include <winsock2.h>
#endif

#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>

#include "lattice_config.h"
#include "struct.h"
#include "socket.h"
#include "serversocket.h"
#include "globals.h"
#include "neighbors.h"
#include "send.h"
#include "macros.h"
#include "server_commands.h"
#include "client_commands.h"

struct message s_mestab[] = {
//    { "whoson", s_whoson, FLAG_REG },
//    { "server", s_server, 0 },
//    { "serverack", s_serverack, 0 },
//    { "kill", s_kill, 0 },
    { "ping", s_ping, 0 },
    { "p", s_p, 0 },
    { "quit", s_quit, 0 },
    { "pc", s_pc, 0 },
    { "pr", s_pr, 0 },
    { "ph", s_ph, 0 },
    { "chat", s_chat, 0 },
    { "action", s_action, 0 },
    { "s", s_s, 0 },
    { "sc", s_sc, 0 },
    { "bo", s_bo, 0 },
    { "mo", s_mo, 0 },
    { "badd", s_badd, 0 },
    { "bset", s_bset, 0 },
    { "brem", s_brem, 0 },
    { (char *) NULL, (int (*)()) NULL, 0 }
};

struct message *find_message (struct message *tab, char *command) {
    struct message *m = (struct message *)NULL;

    if (!tab || !command) return NULL;

    for (m = tab; m->cmd; m++)
        if (!strcasecmp (command, m->cmd))
            break;

    return m;
}


char *arg_v[MAX_ARGS];
int   arg_c;

void strtoargv(char *line) {

    char *p;

    arg_c = 0;

    if (!line) {
        arg_v[0] = NULL;
        return;
    }

    p = line;

    while(p) {  // clean me up later

        while (*p && isspace(*p)) p++;

        if (*p) arg_v[arg_c] = *p == ':' ? p + 1 : p;
        else break;

        if (*p == ':') while (*p && *p != '\r' && *p != '\n') p++;
        else while (*p && !isspace(*p)) p++;

        arg_c++;

        if (*p) *p = '\0';
        else break;

        p++;

    }

    arg_v[arg_c] = NULL;

    return;

}


int lattice_init(int in_sock, void (*callback)()) {

    int tab_size;
	
	#ifdef __linux__
		struct rlimit limit_info;
		
		if (getrlimit (RLIMIT_NOFILE, &limit_info) < 0)
			return -1;
		// FIXME
		int fdcount = limit_info.rlim_cur;
		
	#else
		// limit from cstdlib
		const int fdcount = 512;
	#endif

    if (!callback)
        return -1;

    gcallback = callback;

    tab_size = fdcount * sizeof(server_socket);

    socket_table = malloc(tab_size);

    if (!socket_table)
         return -1;

    memset(socket_table, 0, tab_size);

    init_neighbor_table();

    FD_ZERO(&rtest_set);
    FD_ZERO(&rready_set);
    FD_ZERO(&wtest_set);
    FD_ZERO(&wready_set);

    if (in_sock > -1) {
        input_sock = in_sock;
        FD_SET(input_sock, &rtest_set);

        if(input_sock > maxfd)
            maxfd = input_sock;

    }

    return 0;

}

int lattice_select(struct timeval *ptimeout) {

    memcpy(&rready_set, &rtest_set, sizeof (rtest_set));
    memcpy(&wready_set, &wtest_set, sizeof (wtest_set));

    return select(maxfd + 1, &rready_set, &wready_set, NULL, ptimeout);

}

void lattice_flush() {
    flushall_write();
    return;
}

void lattice_process() {

    server_socket *s;
    int fd;

    char read_block[READ_LENGTH];

    ssize_t read_length;
    size_t read_move_length;
    size_t read_index;
    size_t check_length;
    char *p;

    struct message *find_command;

    uint32_t from;
    uint32_t *pfrom;

    gettimeofday(&now, NULL);


    for(fd = 0; fd <= maxfd; fd++) {

        s = socket_table + fd;


        if ((fd != input_sock) && (FD_ISSET(fd, &wready_set)))
            if(sendq_flush(s))
                FD_CLR(fd, &wtest_set);


        if ((fd != input_sock) && (FD_ISSET(fd, &rready_set))) {

            read_length = read(fd, read_block, READ_LENGTH);

            if (read_length < 0) {
                if (sock_ignoreError(sock_getError())) continue;
                // there was a problem with the socket
                SetFlagClosed(s);
                closesock(s);
                continue;
            }


            if (!read_length) {
                // EOF
                SetFlagClosed(s);
                closesock(s);
                continue;
            }

            read_index = 0;

            while(read_index < read_length) {

                check_length = my_min(MTU - s->rlen,      // whats left for a full message
                                      read_length - read_index);  // whats left in the buffer
                p = memchr(read_block + read_index, '\n', check_length);

                if (!p && s->rlen + check_length == MTU) {  // MTU violation
                    closesock(s);
                    break;
                }

                if (p) read_move_length = (p - (read_block + read_index)) + 1;
                else read_move_length = check_length;

                if (read_move_length > 0)
                    memcpy(s->rmsg + s->rlen, read_block + read_index, read_move_length);

                read_index += read_move_length;
                s->rlen += read_move_length;


                if (p) {
                    s->rmsg[s->rlen] = '\0';

                    strtoargv(s->rmsg);
                    if (!arg_c) continue;


                    if (isalpha(*arg_v[0]))
                        find_command = find_message(s_mestab, arg_v[0]);
                    else if (isdigit(*arg_v[0]) || (*arg_v[0]=='-'))
                        find_command = find_message(s_mestab, arg_v[1]);
                    else
                        find_command = NULL;

                    if (!find_command) {s->rlen = 0; continue;}

                    if (!find_command->func) {s->rlen = 0; continue;}

/*
                    if ( (find_command->flags & FLAG_REG) && (!TstFlagReg(s)) ) {
                        closesock(s,NOSLIDEOVER, NUM_PROTO, numstr(NUM_PROTO));
                        //s->rlen = 0; // done in closesock()
                        break;
                    }

*/

                    if (isalpha(*arg_v[0])) {

                        if ((*find_command->func)(s, arg_c - 1, arg_v + 1)) break;

                    } else if (isdigit(*arg_v[0]) || (*arg_v[0]=='-')) {

                        if (isdigit(*arg_v[0])) {
                            from = atoi(arg_v[0]);
                            pfrom = &from;
                        } else {
                            pfrom = NULL;
                        }
                        if ((*find_command->func)(s, pfrom, arg_c - 2, arg_v + 2)) break;
                    }

                    s->rlen = 0;

                }  // end p

            } // while(read_index < read_length)

        } // End if(FD_ISSET(fd &rready_set))

    } // End for(fd = 0; fd <= fdmax; fd++)
    return;
}

int lattice_send(lattice_message *msg) {

    if (!msg) return -1;

    switch (msg->type) {

        case T_P:
            return c_p( ((lattice_p *)msg->args)->wcoord,
                        ((lattice_p *)msg->args)->bcoord );
        break;

        case T_QUIT:
            return c_quit( (char *)msg->args );
        break;

        case T_PC:
            return c_pc( ((lattice_pc *)msg->args)->color );
        break;

        case T_PR:
            return c_pr( ((lattice_pr *)msg->args)->rot );
        break;

        case T_PH:
            return c_ph( ((lattice_ph *)msg->args)->hand );
        break;

        case T_CHAT:
            return c_chat( (char *)msg->args );
        break;

        case T_ACTION:
            return c_action( (char *)msg->args );
        break;

        case T_S:
            return c_s( ((lattice_s *)msg->args)->mid,
                        ((lattice_s *)msg->args)->sid );
        break;

        case T_SC:
            return c_sc( ((lattice_sc *)msg->args)->csid );
        break;

        case T_BO:
            return c_bo( ((lattice_bo *)msg->args)->wcoord,
                         ((lattice_bo *)msg->args)->bcoord,
                         ((lattice_bo *)msg->args)->id );
        break;

        case T_MO:
            return c_mo( ((lattice_mo *)msg->args)->wcoord,
                         ((lattice_mo *)msg->args)->bcoord,
                         ((lattice_mo *)msg->args)->id,
                         ((lattice_mo *)msg->args)->count );
        break;

        case T_BADD:
            return c_badd( ((lattice_badd *)msg->args)->wcoord,
                           ((lattice_badd *)msg->args)->bcoord,
                           ((lattice_badd *)msg->args)->block );
        break;

        case T_BSET:
            return c_bset( ((lattice_bset *)msg->args)->wcoord,
                           ((lattice_bset *)msg->args)->bcoord,
                           ((lattice_bset *)msg->args)->block );
        break;

        case T_BREM:
            return c_brem( ((lattice_brem *)msg->args)->wcoord,
                           ((lattice_brem *)msg->args)->bcoord );
        break;


        default:
            return -1;
        break;



    }

    return 0;

}
