#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include <ctype.h>

#ifdef __linux__
#include <sys/time.h>
#include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <errno.h>
    #include <sys/resource.h>
    #include <netdb.h>
#else
    #include <winsock2.h>
    #include <windows.h>
    #include <ws2tcpip.h>
    #include <stdio.h>
    #ifndef __MINGW32__
        #define strcasecmp(a, b) _stricmp((a), (b))
        #include "forwin.h"
        #include "liblattice.h"
    #else
        #include <sys/time.h>
        #include <unistd.h>
    #endif
#endif


#include "lattice_config.h"
#include "socket.h"
#include "serversocket.h"
#include "struct.h"
#include "globals.h"
#include "neighbors.h"
#include "send.h"
#include "macros.h"
#include "server_commands.h"
#include "client_commands.h"
#include "sched.h"

struct message s_mestab[] = {
    { "ping", s_ping, FLAG_REG },
    { "pong", s_pong, FLAG_REG },
    { "iamserver", s_iamserver, 0 },
    { "p", s_p, FLAG_REG },
    { "quit", s_quit, FLAG_REG },
    { "pc", s_pc, FLAG_REG },
    { "pr", s_pr, FLAG_REG },
    { "ph", s_ph, FLAG_REG },
    { "chat", s_chat, FLAG_REG },
    { "action", s_action, FLAG_REG },
    { "s", s_s, FLAG_REG },
    { "sc", s_sc, FLAG_REG },
    { "bo", s_bo, FLAG_REG },
    { "mo", s_mo, FLAG_REG },
    { "badd", s_badd, FLAG_REG },
    { "bset", s_bset, FLAG_REG },
    { "brem", s_brem, FLAG_REG },
    { "pmine", s_pmine, FLAG_REG },
    { "schat", s_schat, FLAG_REG },
    { "log", s_log, FLAG_REG },
    { "satstep", s_satstep, FLAG_REG },
    { "sat", s_sat, FLAG_REG },
    { "fade", s_fade, FLAG_REG },
    { "user", s_user, FLAG_REG },
    { "server", s_server, FLAG_REG },
    { "delserver", s_delserver, FLAG_REG },
    { "moveto", s_moveto, FLAG_REG },
    { "movefrom", s_movefrom, FLAG_REG },
    { "trackerfailure", s_trackerfailure, FLAG_REG },
    { "closing", s_closing, FLAG_REG },
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


int lattice_init(int in_sock, void (*callback)(lattice_message *mp)) {

    int tab_size;

    #ifdef __linux__
        struct rlimit limit_info;

        if (getrlimit (RLIMIT_NOFILE, &limit_info) < 0)
            return -1;
        // FIXME
        int fdcount = limit_info.rlim_cur;
    #else
        // limit from cstdlib
        const int fdcount = 65536;
    #endif

    #ifdef _WIN32

        WORD wversion = MAKEWORD(2, 2);
        WSADATA wsaData;

        if(WSAStartup(wversion, &wsaData) != 0)
            return -1;

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

    sched.head = NULL;
    sched.tail = NULL;

    return 0;

}

int lattice_select(struct timeval *pt) { // pointer to timeout

    struct timeval sd, *psd = NULL; // scheduler delay, pointer to scheduler delay

    struct timeval *pd = NULL; // pointer to final delay

    memcpy(&rready_set, &rtest_set, sizeof (rtest_set));
    memcpy(&wready_set, &wtest_set, sizeof (wtest_set));

    gettimeofday(&now, NULL);

    psd = sched_whens_next(&sched, &sd);

    if (psd) {
        if (pt) {
            if (timercmp(psd, pt, <))
                pd = psd;
            else
                pd = pt;
        } else {
            pd = psd;
        }
    } else {
        if (pt) {
            pd = pt;
        } else {
            pd = NULL;
        }
    }

    return select(maxfd + 1, &rready_set, &wready_set, NULL, pd);

}

void lattice_flush(void) {
    flushall_write();
    return;
}

void lattice_process(void) {

    server_socket *s;
    int fd;

    char read_block[READ_LENGTH];

    int read_length;
    size_t read_move_length;
    size_t read_index;
    size_t check_length;
    char *p;

    struct message *find_command;

    uint32_t from;
    uint32_t *pfrom;

    gettimeofday(&now, NULL);

    sched_process_all(&sched);

    for(fd = 0; fd <= maxfd; fd++) {

        s = socket_table + fd;

        if ((fd != input_sock) && (FD_ISSET(fd, &wready_set)))
            if(sendq_flush(s))
                FD_CLR(fd, &wtest_set);

        if ((fd != input_sock) && (FD_ISSET(fd, &rready_set))) {

            //read_length = read(fd, read_block, READ_LENGTH);
            read_length = recv(fd, read_block, READ_LENGTH, 0);

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


                    if ( (find_command->flags & FLAG_REG) && (!TstFlagReg(s)) ) {
                        closesock(s);
                        //s->rlen = 0; // done in closesock()
                        break;
                    }



                    if (isalpha(*arg_v[0])) {

                        if ((*find_command->func)(s, NULL, arg_c - 1, arg_v + 1)) break;

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

            if (msg->args)
                return c_p( ((lattice_p *)msg->args)->wcoord,
                            ((lattice_p *)msg->args)->bcoord );
            else
                return c_p_empty();

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

        case T_PMINE:
            return c_pmine( ((lattice_pmine *)msg->args)->mining );
        break;

        case T_SCHAT:
            return c_schat( (char *)msg->args );
        break;

        case T_LUSERS:
            return c_lusers();
        break;

        default:
            return -1;
        break;



    }

    return 0;

}


int lattice_connect(char *ipstr, uint16_t port) {

    struct in_addr ip;
    struct hostent *he;

    server_socket *p;

    lattice_message mess;

    if (!ipstr || !*ipstr) return -2;

    if (!lattice_player.nickname || !*lattice_player.nickname) return -3;

    he = gethostbyname (ipstr);

    if (he) {
        if (*he->h_addr_list)
            memcpy((char *) &ip, *he->h_addr_list, sizeof(ip));
        else
            ip.s_addr = inet_addr(ipstr);
    } else {
        ip.s_addr = inet_addr(ipstr);
    }

    p = connect_server(lattice_player.centeredon, ip, port);

    if (!p) return -4;

    neighbor_table[1][1][1]=p;

    mess.type = T_CONNECTED;
    ClrFlagFrom(&mess);
    mess.fromuid = 0;
    mess.length = 0;
    mess.args = NULL;

    (*gcallback)(&mess);

    return (sendto_one(neighbor_table[1][1][1],
                       //                              wx  wy  wz bx by bz  HEAD  HAND
                       "CENTEREDINTRO %lu %d %u %s %d %u %u %u %d %d %d %d %d %d %d %d %u\n",
                       lattice_player.userid,
                       lattice_player.model,
                       lattice_player.color,
                       lattice_player.nickname,
                       lattice_player.burstdist,
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
                       lattice_player.usercolor
                       ));


}


int lattice_setplayer(lattice_player_t *player) {

    if (!player) return -1;
    if (!player->nickname) return -2;
    if (!*player->nickname) return -3;

    lattice_player.userid = player->userid;
    lattice_player.flags = player->flags;
    lattice_player.model = player->model;
    lattice_player.color = player->color;
    lattice_player.wpos.x = player->wpos.x;
    lattice_player.wpos.y = player->wpos.y;
    lattice_player.wpos.z = player->wpos.z;
    lattice_player.bpos.x = player->bpos.x;
    lattice_player.bpos.y = player->bpos.y;
    lattice_player.bpos.z = player->bpos.z;
    lattice_player.hrot.xrot = player->hrot.xrot;
    lattice_player.hrot.yrot = player->hrot.yrot;
    lattice_player.hhold.item_id = player->hhold.item_id;
    lattice_player.hhold.item_type = player->hhold.item_type;
    lattice_player.usercolor = player->usercolor;
    lattice_player.burstdist = player->burstdist;
    lattice_player.mining = player->mining;

    if (lattice_player.nickname) free(lattice_player.nickname);

    lattice_player.nickname = strdup(player->nickname);

    lattice_player.centeredon.x = lattice_player.wpos.x >> 8;
    lattice_player.centeredon.y = lattice_player.wpos.y >> 8;
    lattice_player.centeredon.z = lattice_player.wpos.z >> 8;
    lattice_player.my_min_wcoord.x = lattice_player.centeredon.x << 8;
    lattice_player.my_min_wcoord.y = lattice_player.centeredon.y << 8;
    lattice_player.my_min_wcoord.z = lattice_player.centeredon.z << 8;
    lattice_player.my_max_wcoord.x = (lattice_player.centeredon.x << 8) | 0x000000FF;
    lattice_player.my_max_wcoord.y = (lattice_player.centeredon.y << 8) | 0x000000FF;
    lattice_player.my_max_wcoord.z = (lattice_player.centeredon.z << 8) | 0x000000FF;

    return 0;

}

