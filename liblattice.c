#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <stdio.h>

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
    //#include <stdio.h>
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
#include "lattice_packet.h"

struct message s_mestab[] = {
    { T_PING, s_ping, FLAG_REG },
    { T_PONG, s_pong, FLAG_REG },
    { T_IAMSERVER, s_iamserver, 0 },
    { T_P, s_p, FLAG_REG },
    { T_QUIT, s_quit, FLAG_REG },
    { T_PC, s_pc, FLAG_REG },
    { T_PR, s_pr, FLAG_REG },
    { T_PH, s_ph, FLAG_REG },
    { T_CHAT, s_chat, FLAG_REG },
    { T_ACTION, s_action, FLAG_REG },
    { T_S, s_s, FLAG_REG },
    { T_SC, s_sc, FLAG_REG },
    { T_BO, s_bo, FLAG_REG },
    { T_MO, s_mo, FLAG_REG },
    { T_BADD, s_badd, FLAG_REG },
    { T_BSET, s_bset, FLAG_REG },
    { T_BREM, s_brem, FLAG_REG },
    { T_PMINE, s_pmine, FLAG_REG },
    { T_SCHAT, s_schat, FLAG_REG },
    { T_LOG, s_log, FLAG_REG },
    { T_SATSTEP, s_satstep, FLAG_REG },
    { T_SAT, s_sat, FLAG_REG },
    { T_FADE, s_fade, FLAG_REG },
    { T_USER, s_user, FLAG_REG },
    { T_SERVER, s_server, FLAG_REG },
    { T_DELSERVER, s_delserver, FLAG_REG },
    { T_MOVETO, s_moveto, FLAG_REG },
    { T_MOVEFROM, s_movefrom, FLAG_REG },
    { T_TRACKERFAILURE, s_trackerfailure, FLAG_REG },
    { T_CLOSING, s_closing, FLAG_REG },
    { 0, (int (*)()) NULL, 0 }
};

/*
struct message *find_message (struct message *tab, char *command) {
    struct message *m = (struct message *)NULL;

    if (!tab || !command) return NULL;

    for (m = tab; m->cmd; m++)
        if (!strcasecmp (command, m->cmd))
            break;

    return m;
}
*/

struct message *find_message (struct message *tab, int type) {
    struct message *m = (struct message *)NULL;

    if (!tab) return NULL;

    for (m = tab; m->type; m++)
        if (m->type == type)
            if (m->func) return m;

    return NULL;
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

    if (lattice_initialized) return -1;
    if (lattice_connected) return -1;

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

    lattice_initialized = 1;

    return 0;

}

int lattice_select(struct timeval *pt) { // pointer to timeout

    struct timeval sd, *psd = NULL; // scheduler delay, pointer to scheduler delay

    struct timeval *pd = NULL; // pointer to final delay

    if (!lattice_initialized) return -1;
    if (!lattice_connected) return -1;

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
    if (!lattice_initialized) return;
    if (!lattice_connected) return;
    flushall_write();
    return;
}

void lattice_disconnect(void) {
    if (!lattice_initialized) return;
    if (!lattice_connected) return;
    disconnect_servers();
    return;
}

void lattice_process(void) {

    server_socket *s;
    int fd;

    char read_block[READ_LENGTH];

    int read_length;
    size_t read_move_length;
    size_t read_index;
    //size_t check_length;
    //char *p;

    struct message *find_command;

//    uint32_t from;
//    uint32_t *pfrom;

    lt_packet *packet;

    if (!lattice_initialized) return;

    if (!lattice_connected) return;

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

            // BINARY based protocols
            read_index = 0;

            while(read_index < read_length) {

                if(s->rlen < sizeof(lt_packet_h)) {
                    // we do not yet have the main header for this packet

                    read_move_length = my_min(
                        sizeof(lt_packet_h) - s->rlen,
                        read_length - read_index );

                    if (read_move_length > 0)
                        memcpy((char *)s->rmsg + s->rlen,
                            read_block + read_index,
                            read_move_length);

                    read_index += read_move_length;
                    s->rlen += read_move_length;

                    if(s->rlen < sizeof(lt_packet_h)) break;

                    packet = (lt_packet *)s->rmsg;

                    packet_ntoh(&packet->header);

                } else {

                    packet = (lt_packet *)s->rmsg;

                }

                // at this point we know we have a header in host order

                if (packet->header.marker != SYNCH_MARKER) {
                    // This is a loss of synchronization. close the socket
                    closesock(s);
                    break;
                }

                if ( packet->header.payload_length > PAYLOAD_MTU) {
                    // This is an MTU violation. close the socket
                    closesock(s);
                    break;
                }

                read_move_length = my_min(
                    packet->header.payload_length - (s->rlen - sizeof(lt_packet_h)),
                    read_length - read_index );

                if (read_move_length > 0)
                    memcpy((char *)s->rmsg + s->rlen,
                        read_block + read_index,
                        read_move_length);

                read_index += read_move_length;
                s->rlen += read_move_length;

                if( s->rlen <
                    (sizeof(lt_packet_h) + packet->header.payload_length) )
                    break;

                // We have a packet ready to execute!

                //switch(s->type) {

                //    case SOCKET_SERVER:
                //        find_packet_command = find_packet_message(ps_mestab, packet->header.payload_type);
                //    break;
                //    default:
                //        find_packet_command = NULL;
                //    break;
                //}

                find_command = find_message(s_mestab, packet->header.payload_type);

                if (!find_command) {s->rlen = 0; continue;}

                if (!find_command->func) {s->rlen = 0; continue;}

                if ( (find_command->flags & FLAG_REG) && (!TstFlagReg(s)) ) {
                    closesock(s);
                    //s->rlen = 0; // done in closesock()
                    break;
                }

                if ((*find_command->func)(s, packet)) break; // if true we've been closed, dont process more...

                s->rlen=0;

            } // End while(read_index < read_length)

/*
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
*/

        } // End if(FD_ISSET(fd &rready_set))

    } // End for(fd = 0; fd <= fdmax; fd++)
    return;
}

int lattice_send(lattice_message *msg) {

    if (!msg) return -1;

    if (!lattice_initialized) return -1;

    if (!lattice_connected) return -1;

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
            return c_chat( ((lattice_chat *)msg->args)->chat_text );
        break;

        case T_PCHAT:
            return c_pchat( ((lattice_pchat *)msg->args)->uid, ((lattice_pchat *)msg->args)->pchat_text );
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


int lattice_connect(const char *ipstr, uint16_t port) {

    struct in_addr ip;
    struct hostent *he;

    void *p;
    lt_packet out_packet;

    int error;

    server_socket *dst;

    lattice_message mess;

    if (!ipstr || !*ipstr) return -10;

    if (!lattice_player.nickname || !*lattice_player.nickname) return -11;

    if (!lattice_initialized) return -32;

    if (lattice_connected) return -33;

    he = gethostbyname (ipstr);

    if (he) {
        if (*he->h_addr_list)
            memcpy((char *) &ip, *he->h_addr_list, sizeof(ip));
        else
            return -30;
            //ip.s_addr = inet_addr(ipstr);
    } else {
        return -31;
        //ip.s_addr = inet_addr(ipstr);
    }

    dst = connect_server(lattice_player.centeredon, ip, port, &error);

    if (!dst) return error;

    neighbor_table[1][1][1]=dst;

    mess.type = T_CONNECTED;
    ClrFlagFrom(&mess);
    mess.fromuid = 0;
    mess.length = 0;
    mess.args = NULL;

    (*gcallback)(&mess);

    makepacket(&out_packet, T_CENTEREDINTRO);
    p = &out_packet.payload;

    if (!put_uid(&p, lattice_player.userid, &PLength(&out_packet), &PArgc(&out_packet))) return -13;
    if (!put_model(&p, lattice_player.model, &PLength(&out_packet), &PArgc(&out_packet))) return -14;
    if (!put_color(&p, lattice_player.color, &PLength(&out_packet), &PArgc(&out_packet))) return -15;
    if (!put_nickname(&p, lattice_player.nickname, &PLength(&out_packet), &PArgc(&out_packet))) return -16;
    if (!put_burstdist(&p, lattice_player.burstdist, &PLength(&out_packet), &PArgc(&out_packet))) return -17;
    if (!put_wx(&p, lattice_player.wpos.x, &PLength(&out_packet), &PArgc(&out_packet))) return -18;
    if (!put_wy(&p, lattice_player.wpos.y, &PLength(&out_packet), &PArgc(&out_packet))) return -19;
    if (!put_wz(&p, lattice_player.wpos.z, &PLength(&out_packet), &PArgc(&out_packet))) return -20;
    if (!put_bx(&p, lattice_player.bpos.x, &PLength(&out_packet), &PArgc(&out_packet))) return -21;
    if (!put_by(&p, lattice_player.bpos.y, &PLength(&out_packet), &PArgc(&out_packet))) return -22;
    if (!put_bz(&p, lattice_player.bpos.z, &PLength(&out_packet), &PArgc(&out_packet))) return -23;
    if (!put_xrot(&p, lattice_player.hrot.xrot, &PLength(&out_packet), &PArgc(&out_packet))) return -24;
    if (!put_yrot(&p, lattice_player.hrot.yrot, &PLength(&out_packet), &PArgc(&out_packet))) return -25;
    if (!put_item_id(&p, lattice_player.hhold.item_id, &PLength(&out_packet), &PArgc(&out_packet))) return -26;
    if (!put_item_type(&p, lattice_player.hhold.item_type, &PLength(&out_packet), &PArgc(&out_packet))) return -27;
    if (!put_mining(&p, lattice_player.mining, &PLength(&out_packet), &PArgc(&out_packet))) return -28;
    if (!put_usercolor(&p, lattice_player.usercolor, &PLength(&out_packet), &PArgc(&out_packet))) return -29;

    if (sendpacket(neighbor_table[1][1][1], &out_packet)) return -12;

    lattice_connected = 1;

    //if(sendto_one(neighbor_table[1][1][1],
    //                   //                              wx  wy  wz bx by bz  HEAD  HAND
    //                   "CENTEREDINTRO %lu %d %u %s %d %u %u %u %d %d %d %d %d %d %d %d %u\n",
    //                   lattice_player.userid,
    //                   lattice_player.model,
    //                   lattice_player.color,
    //                   lattice_player.nickname,
    //                   lattice_player.burstdist,
    //                   lattice_player.wpos.x,
    //                   lattice_player.wpos.y,
    //                   lattice_player.wpos.z,
    //                   lattice_player.bpos.x,
    //                   lattice_player.bpos.y,
    //                   lattice_player.bpos.z,
    //                   lattice_player.hrot.xrot,
    //                   lattice_player.hrot.yrot,
    //                   lattice_player.hhold.item_id,
    //                   lattice_player.hhold.item_type,
    //                   lattice_player.mining,
    //                   lattice_player.usercolor
    //                   )) return -12;

    return 0;

}


int lattice_setplayer(lattice_player_t *player) {

    if (!player) return -1;
    if (!player->nickname) return -2;
    if (!*player->nickname) return -3;
    if (!lattice_initialized) return -4;
    if (lattice_connected) return -5;

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

    //if (lattice_player.nickname) free(lattice_player.nickname);

    //lattice_player.nickname = strdup(player->nickname);
    strncpy(lattice_player.nickname, player->nickname, sizeof(lattice_player.nickname));
    lattice_player.nickname[sizeof(lattice_player.nickname)-1]='\0';

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


int lattice_getplayer(lattice_player_t *player) {

    if (!player) return -1;
    if (!lattice_initialized) return -2;
    if (!lattice_connected) return -3;

    player->userid = lattice_player.userid;
    player->flags = lattice_player.flags;
    player->model = lattice_player.model;
    player->color = lattice_player.color;
    player->wpos.x = lattice_player.wpos.x;
    player->wpos.y = lattice_player.wpos.y;
    player->wpos.z = lattice_player.wpos.z;
    player->bpos.x = lattice_player.bpos.x;
    player->bpos.y = lattice_player.bpos.y;
    player->bpos.z = lattice_player.bpos.z;
    player->hrot.xrot = lattice_player.hrot.xrot;
    player->hrot.yrot = lattice_player.hrot.yrot;
    player->hhold.item_id = lattice_player.hhold.item_id;
    player->hhold.item_type = lattice_player.hhold.item_type;
    player->usercolor = lattice_player.usercolor;
    player->burstdist = lattice_player.burstdist;
    player->mining = lattice_player.mining;

    //if (player->nickname) free(player->nickname);

    //player->nickname = strdup(lattice_player.nickname);
    strncpy(player->nickname, lattice_player.nickname, sizeof(player->nickname));
    player->nickname[sizeof(player->nickname)-1]='\0';

    player->centeredon.x = player->wpos.x >> 8;
    player->centeredon.y = player->wpos.y >> 8;
    player->centeredon.z = player->wpos.z >> 8;
    player->my_min_wcoord.x = player->centeredon.x << 8;
    player->my_min_wcoord.y = player->centeredon.y << 8;
    player->my_min_wcoord.z = player->centeredon.z << 8;
    player->my_max_wcoord.x = (player->centeredon.x << 8) | 0x000000FF;
    player->my_max_wcoord.y = (player->centeredon.y << 8) | 0x000000FF;
    player->my_max_wcoord.z = (player->centeredon.z << 8) | 0x000000FF;

    return 0;

}

// --------------------------------------------------

inline int authserver_end(int sockfd, int returnValue)
{
	// close socket
	#ifdef __linux__
		close(sockfd);
	#else
		closesocket(sockfd);
	#endif
	return returnValue;
}

const char* authserver_errorstring(int error)
{
	switch (error)
	{
	case 0:
		return "No error";
	case -1:
		return "Missing username";
	case -2:
		return "Missing password";
	case -3:
		return "Missing hostname";
	case -4:
	case -5:
		return "Hostname unresolved";
		
	case -6:
		return "Could not create socket";
	case -7:
		return "Could not connect to auth server";
	case -8:
		return "Could not read from socket";
	case -9:
		return "Authserver closed connection early";
		
	case -10:
		return "Missing arguments from auth server";
	case -11:
		return "Received incorrect command token";
	case -12:
		return "Could not send data to authserver";
		
	case -13:
		return "Authentication failed";
	case -14:
		return "Could not connect to lattice server";
		
	case -20:
		return "LibLattice not initialized";
	case -21:
		return "Already connected to lattice";
	}
	char buf[64];
	sprintf(buf, "Unknown error: %d", error); // puts string into buffer	
	return strdup(buf);
}

void authserver_printerror(const char* baseString)
{
	#ifdef _WIN32
		printf(baseString, WSAGetLastError());
	#else
		printf(baseString, strerror(errno));
	#endif
}

int authserver_login(const char *username, const char *password, const char *hostname, uint16_t port, uint16_t burstdist) {

    struct in_addr ip;
    struct hostent *he;
    int sockfd;
    struct sockaddr_in  serv_addr;
	
    lattice_player_t player;
	uint16_t seed_port;
	
    if (!username || !*username) return -1;
    if (!password || !*password) return -2;
    if (!hostname || !*hostname) return -3;

    if (!lattice_initialized) return -20;
    if (lattice_connected) return -21;

    he = gethostbyname (hostname);
	
    if (he) {
        if (*he->h_addr_list)
            memcpy((char *) &ip, *he->h_addr_list, sizeof(ip));
        else
            //ip.s_addr = inet_addr(hostname);
            return -4;
    } else {
        //ip.s_addr = inet_addr(hostname);
        return -5;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd < 0) {
        return -6;
    }

    memset(&serv_addr, 0, sizeof serv_addr);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = ip.s_addr;
	
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof serv_addr) < 0)
	{
		return authserver_end(sockfd, -7);
    }
	
	// peek for data
	#define READ_BUFLEN 1024
	char readBuffer[READ_BUFLEN];
	int bytesRead = recv(sockfd, readBuffer, READ_BUFLEN, 0);
	
	if (bytesRead == SOCKET_ERROR)
	{
		// something bad happened
		authserver_printerror("read failed: %d\n");
		return authserver_end(sockfd, -8);
	}
	else if (bytesRead == 0)
	{
		// connection closed early?
		return authserver_end(sockfd, -9);
	}
	
    strtoargv(readBuffer);
	// missing arguments
	if (arg_c < 1) return authserver_end(sockfd, -10);
	// not correct command token
	if (strcasecmp(arg_v[0], "AUTHSERVER")) return authserver_end(sockfd, -11);
	
	// send response
	snprintf(readBuffer, READ_BUFLEN, "LOGIN %s %s\n", username, password);
	
	int sendBytes = send(sockfd, readBuffer, (int) strlen(readBuffer), 0);
    if (sendBytes == SOCKET_ERROR)
	{
        authserver_printerror("send failed: %d\n");
        return authserver_end(sockfd, -12);
    }
	else if (sendBytes == 0)
	{
		// ?? what now ??
	}
	
	// we no longer need to send data
	shutdown(sockfd, SD_SEND);
	
	// wait for response
	bytesRead = recv(sockfd, readBuffer, READ_BUFLEN, 0);
	
	if (bytesRead == SOCKET_ERROR)
	{
		// something bad happened
		authserver_printerror("read failed: %d\n");
		return authserver_end(sockfd, -8);
	}
	else if (bytesRead == 0)
	{
		// connection closed early?
		return authserver_end(sockfd, -9);
	}
	
    strtoargv(readBuffer);
	// missing arguments
    if (arg_c < 17) return -10;
	// authentication failed
	if (strcasecmp(arg_v[0], "AUTHOK")) return -13;
	
    //AUTHOK username uid model color wx wy wz bx by bz hrot_x hrot_y hhold_id hhold_type host port

    //nickname = strdup(arg_v[1]);
    //if (!nickname) return -19;
    //player.nickname = nickname;
    strncpy(player.nickname, arg_v[1], sizeof(player.nickname));
    player.nickname[sizeof(player.nickname)-1]='\0';

    player.userid = (uint32_t)atoi(arg_v[2]);
    player.model = (uint16_t)atoi(arg_v[3]);
    player.color = (uint32_t)atoi(arg_v[4]);
    player.wpos.x = (int32_t)atoi(arg_v[5]);
    player.wpos.y = (int32_t)atoi(arg_v[6]);
    player.wpos.z = (int32_t)atoi(arg_v[7]);
    player.bpos.x = (int32_t)atoi(arg_v[8]);
    player.bpos.y = (int32_t)atoi(arg_v[9]);
    player.bpos.z = (int32_t)atoi(arg_v[10]);
    player.hrot.xrot = (int16_t)atoi(arg_v[11]);
    player.hrot.yrot = (int16_t)atoi(arg_v[12]);
    player.hhold.item_id = (uint16_t)atoi(arg_v[13]);
    player.hhold.item_type = (uint16_t)atoi(arg_v[14]);
    player.usercolor = 0;
    player.mining = 0;
    player.burstdist = burstdist;
	
    lattice_setplayer(&player);
	
    //free (nickname);
	
    seed_port = (uint16_t)atoi(arg_v[16]);
	
    int ret = lattice_connect(arg_v[15], seed_port);
	if (ret < 0) return -14;
	
    lattice_flush();
	
    return authserver_end(sockfd, 0);
}

