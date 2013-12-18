#include <stdlib.h>
#include <string.h>

#ifdef __linux__
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <arpa/inet.h>
#else
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#endif

#include "lattice_config.h"
#include "serversocket.h"
#include "struct.h"
#include "send.h"
#include "globals.h"
#include "neighbors.h"

int s_ping(struct server_socket *src, int argc, char **argv) {

    if (!src) return 0;

    if (sendto_one(src, "PONG\n")) return 1;

    return 0;

}

int s_p(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;
    lattice_p submess;

    uid_link *uidlink;

    if (!src) return 0;

    if (!pfrom) return 0;

    mess.type = T_P;
    SetFlagFrom(&mess);
    mess.fromuid = *pfrom;

    if (argc < 6) {
        mess.args = NULL;
    } else {
        mess.args = &submess;
        submess.wcoord.x = atoi(argv[0]);
        submess.wcoord.y = atoi(argv[1]);
        submess.wcoord.z = atoi(argv[2]);
        submess.bcoord.x = atoi(argv[3]);
        submess.bcoord.y = atoi(argv[4]);
        submess.bcoord.z = atoi(argv[5]);

        uidlink = uid_link_find(src, mess.fromuid);

        if (uidlink)
            uidlink->standing_on = ncoord_is_equal(src->coord, wcoord_to_ncoord(submess.wcoord));


    }

    (*gcallback)(&mess);

    return 0;

}


int s_quit(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;
    lattice_quit submess;

    uid_link *uidlink;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 2) return 0;

    mess.type = T_QUIT;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    submess.numeric = atoi(argv[0]);
    strncpy(submess.desc, argv[1] , sizeof(submess.desc));
    submess.desc[sizeof(submess.desc)-1]='\0';


    uidlink = uid_link_find(src, mess.fromuid);

    if (uidlink)
        uid_link_del(src, uidlink);

    (*gcallback)(&mess);

    return 0;

}


int s_pc(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_pc submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 1) return 0;

    mess.type = T_PC;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    submess.color = (uint32_t)atoi(argv[0]);

    (*gcallback)(&mess);

    return 0;

}


int s_pr(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_pr submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 2) return 0;

    mess.type = T_PR;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    submess.rot.xrot = (HEAD_TYPE)atoi(argv[0]);
    submess.rot.yrot = (HEAD_TYPE)atoi(argv[1]);

    (*gcallback)(&mess);

    return 0;

}

int s_ph(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_ph submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 2) return 0;

    mess.type = T_PH;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    submess.hand.item_id = (HAND_TYPE)atoi(argv[0]);
    submess.hand.item_type = (HAND_TYPE)atoi(argv[1]);

    (*gcallback)(&mess);

    return 0;

}


int s_chat(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_chat submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 1) return 0;

    mess.type = T_CHAT;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    strncpy(submess.string, argv[0] , sizeof(submess.string));
    submess.string[sizeof(submess.string)-1]='\0';

    (*gcallback)(&mess);

    return 0;

}


int s_action(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_action submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 1) return 0;

    mess.type = T_ACTION;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    strncpy(submess.string, argv[0] , sizeof(submess.string));
    submess.string[sizeof(submess.string)-1]='\0';

    (*gcallback)(&mess);

    return 0;

}

int s_s(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_s submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 2) return 0;

    mess.type = T_S;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    submess.mid = (int32_t)atoi(argv[0]);
    submess.sid = (int32_t)atoi(argv[1]);

    (*gcallback)(&mess);

    return 0;

}

int s_sc(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_sc submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 1) return 0;

    mess.type = T_SC;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    submess.csid = (int32_t)atoi(argv[0]);

    (*gcallback)(&mess);

    return 0;

}


int s_bo(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_bo submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 7) return 0;

    mess.type = T_BO;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    submess.wcoord.x = atoi(argv[0]);
    submess.wcoord.y = atoi(argv[1]);
    submess.wcoord.z = atoi(argv[2]);
    submess.bcoord.x = atoi(argv[3]);
    submess.bcoord.y = atoi(argv[4]);
    submess.bcoord.z = atoi(argv[5]);
    submess.id = atoi(argv[6]);


    (*gcallback)(&mess);

    //if (sendto_one(src, "PONG\n")) return 1;

    return 0;

}


int s_mo(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_mo submess;

    if (!src) return 0;

    if (argc < 8) return 0;

    mess.type = T_MO;

    if (pfrom) {
        SetFlagFrom(&mess);
        mess.fromuid = *pfrom;
    } else {
        ClrFlagFrom(&mess);
        mess.fromuid = 0;
    }

    mess.args = &submess;

    submess.wcoord.x = atoi(argv[0]);
    submess.wcoord.y = atoi(argv[1]);
    submess.wcoord.z = atoi(argv[2]);
    submess.bcoord.x = atoi(argv[3]);
    submess.bcoord.y = atoi(argv[4]);
    submess.bcoord.z = atoi(argv[5]);
    submess.id = atoi(argv[6]);
    submess.count = atoi(argv[7]);


    (*gcallback)(&mess);

    //if (sendto_one(src, "PONG\n")) return 1;

    return 0;

}

int s_badd(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_badd submess;

    if (!src) return 0;

    if (argc < 8) return 0;

    mess.type = T_BADD;

    if (pfrom) {
        SetFlagFrom(&mess);
        mess.fromuid = *pfrom;
    } else {
        ClrFlagFrom(&mess);
        mess.fromuid = 0;
    }

    mess.args = &submess;

    submess.wcoord.x = atoi(argv[0]);
    submess.wcoord.y = atoi(argv[1]);
    submess.wcoord.z = atoi(argv[2]);
    submess.bcoord.x = atoi(argv[3]);
    submess.bcoord.y = atoi(argv[4]);
    submess.bcoord.z = atoi(argv[5]);
    submess.block.id = atoi(argv[6]);
    submess.block.bf = atoi(argv[7]);


    (*gcallback)(&mess);

    //if (sendto_one(src, "PONG\n")) return 1;

    return 0;

}


int s_bset(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_bset submess;

    if (!src) return 0;


    if (argc < 8) return 0;

    mess.type = T_BSET;

    if (pfrom) {
        SetFlagFrom(&mess);
        mess.fromuid = *pfrom;
    } else {
        ClrFlagFrom(&mess);
        mess.fromuid = 0;
    }

    mess.args = &submess;

    submess.wcoord.x = atoi(argv[0]);
    submess.wcoord.y = atoi(argv[1]);
    submess.wcoord.z = atoi(argv[2]);
    submess.bcoord.x = atoi(argv[3]);
    submess.bcoord.y = atoi(argv[4]);
    submess.bcoord.z = atoi(argv[5]);
    submess.block.id = atoi(argv[6]);
    submess.block.bf = atoi(argv[7]);


    (*gcallback)(&mess);

    //if (sendto_one(src, "PONG\n")) return 1;

    return 0;

}


int s_brem(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_brem submess;

    if (!src) return 0;

    if (argc < 6) return 0;

    mess.type = T_BREM;

    if (pfrom) {
        SetFlagFrom(&mess);
        mess.fromuid = *pfrom;
    } else {
        ClrFlagFrom(&mess);
        mess.fromuid = 0;
    }

    mess.args = &submess;

    submess.wcoord.x = atoi(argv[0]);
    submess.wcoord.y = atoi(argv[1]);
    submess.wcoord.z = atoi(argv[2]);
    submess.bcoord.x = atoi(argv[3]);
    submess.bcoord.y = atoi(argv[4]);
    submess.bcoord.z = atoi(argv[5]);

    (*gcallback)(&mess);

    return 0;

}

int s_pmine(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_pmine submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 1) return 0;

    mess.type = T_PMINE;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    submess.mining = ((int)atoi(argv[0]) ? 1 : 0);

    (*gcallback)(&mess);

    return 0;

}

int s_schat(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_schat submess;

    if (!src) return 0;

    if (argc < 3) return 0;

    mess.type = T_SCHAT;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.args = &submess;

    strncpy(submess.nickname, argv[0] , sizeof(submess.nickname));
    submess.nickname[sizeof(submess.nickname)-1]='\0';
    submess.color = (uint32_t)atoi(argv[1]);
    strncpy(submess.string, argv[2] , sizeof(submess.string));
    submess.string[sizeof(submess.string)-1]='\0';

    (*gcallback)(&mess);

    return 0;

}

int s_log(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_log submess;

    if (!src) return 0;

    if (argc < 1) return 0;

    mess.type = T_LOG;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.args = &submess;

    strncpy(submess.string, argv[0] , sizeof(submess.string));
    submess.string[sizeof(submess.string)-1]='\0';

    (*gcallback)(&mess);

    return 0;

}


int s_satstep(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_satstep submess;

    if (!src) return 0;

    if (argc < 1) return 0;

    mess.type = T_SATSTEP;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.args = &submess;

    submess.satstep = (uint32_t)atoi(argv[0]);

    (*gcallback)(&mess);

    return 0;

}

int s_sat(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_sat submess;

    if (!src) return 0;

    if (argc < 1) return 0;

    mess.type = T_SAT;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.args = &submess;

    submess.sat = (double)atof(argv[0]);

    (*gcallback)(&mess);

    return 0;

}


int s_fade(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    uid_link *uidlink;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 0) return 0;

    mess.type = T_FADE;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = NULL;

    uidlink = uid_link_find(src, mess.fromuid);

    if (uidlink)
        uid_link_del(src, uidlink);

    (*gcallback)(&mess);

    return 0;

}

int s_user(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;
    lattice_user submess;

    //uid_link *uidlink;
    int standing;


    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 15) return 0;

    mess.type = T_USER;

    SetFlagFrom(&mess);

    mess.fromuid = *pfrom;

    mess.args = &submess;

    submess.model = (uint16_t) atoi(argv[0]);
    submess.color = (uint32_t) atoi(argv[1]);

    strncpy(submess.nickname, argv[2] , sizeof(submess.nickname));
    submess.nickname[sizeof(submess.nickname)-1]='\0';

    submess.wpos.x = atoi(argv[3]);
    submess.wpos.y = atoi(argv[4]);
    submess.wpos.z = atoi(argv[5]);

    submess.bpos.x = atoi(argv[6]);
    submess.bpos.y = atoi(argv[7]);
    submess.bpos.z = atoi(argv[8]);

    submess.hrot.xrot = atoi(argv[9]);
    submess.hrot.yrot = atoi(argv[10]);

    submess.hhold.item_id = atoi(argv[11]);
    submess.hhold.item_type = atoi(argv[12]);

    submess.mining = atoi(argv[13]);

    submess.usercolor = (uint32_t) atoi(argv[14]);

    standing = ncoord_is_equal(src->coord, wcoord_to_ncoord(submess.wpos));

    uid_link_add_end(src, mess.fromuid, standing);

    //if (!uidlink) return 0;

    (*gcallback)(&mess);

    return 0;

}

int s_server(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    lattice_message mess;

    lattice_server submess;

    server_socket *p;

    if (!src) return 0;

    if (argc < 5) return 0;

    if (!ncoord_is_equal(src->coord, lattice_player.centeredon)) return 0;

    mess.type = T_SERVER;

    ClrFlagFrom(&mess);

    mess.fromuid = 0;

    mess.args = &submess;

    submess.ncoord.x = atoi(argv[0]);
    submess.ncoord.y = atoi(argv[1]);
    submess.ncoord.z = atoi(argv[2]);
    submess.ip.s_addr = inet_addr(argv[3]);
    submess.port = atoi(argv[4]);

    p = connect_server(submess.ncoord, submess.ip, submess.port);

    if (!p) return 0;

    add_neighbor(submess.ncoord, p);

    //(*gcallback)(&mess);

    if (user_is_within_outer_border(lattice_player.wpos, submess.ncoord)) {
        // close enogh to be tracked...
        if (sendto_one(p, "SIDEDINTRO %d %d %u %s %d %u %u %u %u %u %u %d %d %d %d %d %d %d %d %u\n",
                       lattice_player.userid,
                       lattice_player.model,
                       lattice_player.color,
                       lattice_player.nickname,
                       lattice_player.burstdist,
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
                       lattice_player.usercolor)) return 1;

    } else {
        // too far to be tracked... just the basics....
        if (sendto_one(p, "SIDEDINTRO %d %d %u %s %d %u %u %u\n",
                       lattice_player.userid,
                       lattice_player.model,
                       lattice_player.color,
                       lattice_player.nickname,
                       lattice_player.burstdist,
                       lattice_player.centeredon.x,
                       lattice_player.centeredon.y,
                       lattice_player.centeredon.z)) return 1;
    }

    return 0;

}


int s_delserver(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    server_socket *p;

    n_coord coord;

    if (!src) return 0;

    if (argc < 3) return 0;

    if (!ncoord_is_equal(src->coord, lattice_player.centeredon)) return 0;

    coord.x = atoi(argv[0]);
    coord.y = atoi(argv[1]);
    coord.z = atoi(argv[2]);

    p = find_neighbor(coord);

    if (!p) return 0;

    closesock(p);

    return 0;

}

int s_moveto(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    uid_link *uidlink;
    n_coord coord;

    lattice_message mess;
    lattice_quit submess;


    struct server_socket *dst;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 3) return 0;

    coord.x = atoi(argv[0]);
    coord.y = atoi(argv[1]);
    coord.z = atoi(argv[2]);

    uidlink = uid_link_find(src, *pfrom);

    if (uidlink) {

        dst = find_neighbor(coord);

        if (dst) {
            if (!uid_link_find(dst, *pfrom))
                uid_link_add_end(dst, *pfrom, 1);
        } else {
            // user is walking into a blackhole
            mess.type = T_QUIT;
            SetFlagFrom(&mess);
            mess.fromuid = *pfrom;
            mess.args = &submess;
            submess.numeric = 121; // NUM_BLACKHOLE_RECENTER
            strncpy(submess.desc, "User recentering to nonexistent space (blackhole)" , sizeof(submess.desc));
            submess.desc[sizeof(submess.desc)-1]='\0';
            (*gcallback)(&mess);

        }

        uid_link_del(src, uidlink);

    }

    return 0;

}


int s_movefrom(struct server_socket *src, uint32_t *pfrom, int argc, char **argv) {

    uid_link *uidlink;
    n_coord coord;

    struct server_socket *from;

    lattice_message mess;
    lattice_user submess;

    if (!src) return 0;

    if (!pfrom) return 0;

    if (argc < 18) return 0;

    coord.x = atoi(argv[0]);
    coord.y = atoi(argv[1]);
    coord.z = atoi(argv[2]);

    from = find_neighbor(coord);

    if (from) {
        if ((uidlink=uid_link_find(from, *pfrom)))
            uid_link_del(from, uidlink);
    } else {

        mess.type = T_USER;
        SetFlagFrom(&mess);
        mess.fromuid = *pfrom;
        mess.args = &submess;

        submess.model = (uint16_t) atoi(argv[3]);
        submess.color = (uint32_t) atoi(argv[4]);

        strncpy(submess.nickname, argv[5] , sizeof(submess.nickname));
        submess.nickname[sizeof(submess.nickname)-1]='\0';

        submess.wpos.x = atoi(argv[6]);
        submess.wpos.y = atoi(argv[7]);
        submess.wpos.z = atoi(argv[8]);

        submess.bpos.x = atoi(argv[9]);
        submess.bpos.y = atoi(argv[10]);
        submess.bpos.z = atoi(argv[11]);

        submess.hrot.xrot = atoi(argv[12]);
        submess.hrot.yrot = atoi(argv[13]);

        submess.hhold.item_id = atoi(argv[14]);
        submess.hhold.item_type = atoi(argv[15]);

        submess.mining = atoi(argv[16]);

        submess.usercolor = (uint32_t) atoi(argv[17]);

        (*gcallback)(&mess);

    }

    if (!uid_link_find(src, *pfrom))
        uid_link_add_end(src, *pfrom, 1);


    return 0;

}

